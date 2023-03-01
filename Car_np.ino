#include <MsTimer2.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN   37
#define RST_PIN  36
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

//取消宏定义注释进入相应的测试
//#define Car_GO_All       //整体程序运行
//#define Test_Motor   //测试电机，默认正转
//#define Test_Sever   //测试舵机，电机停止转动
//#define Test_Diangan_Value //Wifi发回电感值
#xdefine Test_Card_Read  //测试读卡模块工作是否正常

int Ind1 = A0; // 电感1 
int Ind2 = A1; // 电感2 
int Ind3 = A3; // 电感3 Left
int Ind4 = A2; // 电感4 Right

#define MOTOR1 13   // 电机INR
#define MOTOR2 12   // 电机INF

Servo myservo;

void Interrupt_Init();

int Timer_Count = 0;  //中断计数变量
char Time_12ms_Flag = 0;
char Time_24ms_Flag = 0;
char Time_60ms_Flag = 0;
char Time_480ms_Flag = 0;
char Time3s_Flag = 0; //3s定时标志位，注册失败检测
int Time3s_Count = 0; //3s计数变量

extern unsigned char Car_Number; //使用的小车编号,换车时修改此处,“1”对应1号车
extern unsigned char Server_mid_value[25];  //舵机中值调整数组，分别对应各辆车

char Motor_Test_Flag = 0; //舵机测试标志位
char Card_Read_Flag = 0;  //读卡测试标志位
char result = 0; //Wifi用标志位，为1则Wifi连接成功

char Go_Direction[20] = {3,3,3,3,3,3,3,3,3,3,3,3,3};  //行走方向暂存数组，0为直行，1为左转，2为右转，3为停止
char Car_Go = 1;  //0 运行，1 停止
char Car_Speed = 160;  //运行速度

long Card_Data = 0;  //卡片十进制数据
long Last_Card_Data = 0; //上次卡片数据记录

char Turn_Flag = 0;  //换道标志位，为1使能
long Turn_Counter = 0; //打角延时计数，单位为中断周期

void try_key(MFRC522::MIFARE_Key *key);

void setup() {
    SPI.begin();          // Init SPI bus
    mfrc522.PCD_Init();   // Init MFRC522 card
    Traffic_IO_Init();    //Openmv信号口初始化
      
    pinMode(Ind1, INPUT);     pinMode(Ind2, INPUT);
    pinMode(Ind3, INPUT);     pinMode(Ind4, INPUT);
    Interrupt_Init();
    myservo.attach(10);  myservo.write(90);  // 70 ~ 110
    delay(50);  
    
  #ifdef Car_GO_All    //小车程序整体运行
    Wifi_Initialize(); //Wifi
    Car_Confirm();     //小车注册
  #endif
  
  #ifdef Test_Motor    //测试电机
    noInterrupts(); 
    Motor_Turn_Test();
  #endif
  
  #ifdef Test_Sever    //测试舵机
  Motor_Test_Flag = 1;
  #endif               //中断中的舵机控制程序执行
  
  #ifdef Test_Diangan_Value    //Wifi发回电感值
    Wifi_Initialize();  //Wifi
  #endif
  
  #ifdef Test_Card_Read    //Wifi发回卡片数据
    Wifi_Initialize();  //Wifi
    Card_Read_Flag = 1;
  #endif
}

/**************6ms中断服务函数***************/
ISR(TIMER3_OVF_vect)
{
    TCNT3 = 65161; 
    Timer_Count++;
    Interrupt_3s();
    
    if(Timer_Count%2 == 0)   {Time_12ms_Flag = 1;}
    if(Timer_Count%4 == 0)   {Time_24ms_Flag = 1;}
    if(Timer_Count%10 == 0)  {Time_60ms_Flag = 1;}
    if(Timer_Count%80 == 0)  {Time_480ms_Flag = 1;}
    
    if(Timer_Count == 800)   {Timer_Count = 0;}
}

void loop()
{
    if( (Car_Go == 0 || Motor_Test_Flag == 1)&& Time_12ms_Flag == 1 ) //12ms任务
    {
        Time_12ms_Flag = 0;
        run_get();                    //原始值获取
        run_filter();                 //滤波
        run_guiyi();                  //归一化
     
        if( Turn_Flag == 1 )   //执行转向函数，屏蔽PID
        { 
            if( Turn_Counter == 0 ) { Turn_Flag = 0; }  //避免主函数直接置位,导致程序卡死
            Turn_Counter--; 
            if( Turn_Counter == 0 ) { Turn_Flag = 0; }
         }
        if( Turn_Flag == 0 ) { run_error(); }  //计算偏差 并且赋值
    }
    
    if( (Car_Go == 0 || Card_Read_Flag == 1) && Time_24ms_Flag == 1 )  //24ms任务,发车后执行识别卡函数
    {
        Time_24ms_Flag = 0;
        Get_Card_Number(); //获取卡号
        //if( Card_Data == 324 ) { Traffic_Light_Dispose(); }
    }
    
    if( Time_60ms_Flag == 1 )  //60ms任务
    {
        Time_60ms_Flag = 0;
        serial2_Event(); //数据接收及发送处理
        Path_Planning(); //利用上位机数据进行路线规划
        
        if(Car_Go == 0) { analogWrite(MOTOR1, Car_Speed); digitalWrite(MOTOR2,LOW); } //此处更改小车速度,最大255
        if(Car_Go == 1) { digitalWrite(MOTOR1,HIGH); digitalWrite(MOTOR2,HIGH); } //刹车
        if(Car_Go == 2) { analogWrite(MOTOR1, 50); digitalWrite(MOTOR2,HIGH); } //倒车
    }
    
    if( Time_480ms_Flag == 1 ) //480ms任务
    {
        Time_480ms_Flag = 0;
      #ifdef Test_Diangan_Value    //Wifi发回电感值
        ADC_Value_Send();
      #endif
    }
     //if( Card_Data == 324 ) { Car_Go = 1; }
     //if( Card_Data == 327 ) { Turn(105,63); }  //右转105度，延时63*12（ms）
}

/**************** 换道函数，****************/
/************* angle_turn：转角 ************/
/****** time_count：打角时间，单位为12ms ****/
void Turn(float angle_turn,long time_count)
{
    Turn_Flag = 1;  //标志位置位
    myservo.write(Server_mid_value[Car_Number-1]+angle_turn);   // 70 ~ 110
    Turn_Counter = time_count;
}

void Interrupt_Init()
{
     /********定时器中断配置*********/
     noInterrupts();  
     TCCR3A = 0;
     TCCR3B = 0;
     //timer5_counter = 64911;//65536-(62500*0.01); //0.01s 定时器中断
     TCNT3 = 65161; 
     TCCR3B |= (1<<CS32); 
     TIMSK3 |= (1<<TOIE3); 
     interrupts(); 
}

void Get_Card_Number()
{
     MFRC522::MIFARE_Key k;
     
     if ( mfrc522.PICC_IsNewCardPresent()) //寻找新卡
     {
          if ( mfrc522.PICC_ReadCardSerial()) //选中其中一张卡片
          {
              for (byte i = 0; i < 6; i++) k.keyByte[i] = 0xFF;
              try_key(&k); 
          }
     }
}

void try_key(MFRC522::MIFARE_Key *key)
{
        byte buffer[18];  
        byte block  = 4;
        byte status;
        
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
        if (status == MFRC522::STATUS_OK)
        {
            // Read block
	    byte byteCount = sizeof(buffer);
	    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);

            Card_Data = buffer[0] + 256*buffer[1];
            Position_Return(); //卡片数据更新则上报位置
            Last_Card_Data = Card_Data;
        }
        mfrc522.PICC_HaltA();       // Halt PICC
        mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}
