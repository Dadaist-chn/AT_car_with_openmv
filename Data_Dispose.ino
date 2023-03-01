#include <Arduino.h>
#include <Wire.h>

#define Car_Position_1 0x06
#define Car_Position_2 0x0600

unsigned char Read_Buf[30]; //Wifi数据接收缓存数组
int Address_Field; //地址码缓存
int Data_Length;  //数据长度
int Node[30];  //节点数据缓存
char Node_Num = 0; //节点数量记录
char Status_Inquiry[4]; //状态查询指令暂存
char Register_Confirm = 0; //中心对注册确认，0为否 1为是

/********数据预处理函数********/
void Data_Receive()
{
    Address_Field = int(Read_Buf [2]<<8| Read_Buf [1]);  //地址码(小车为0XXXXXXX)
    Data_Length = int(Read_Buf [4]<<8| Read_Buf [5]);   //数据长度域; 先接收数据的高位，后接收低位
    
    if(Read_Buf[0] == 0xFE && Address_Field == Car_Position_2) //Read_Buf[0]为起始符
    {
        switch(Read_Buf[3])
        {
           case 0x01:  //设置智能汽车参数
               Car_Speed = Read_Buf[6]; 
               for(char i=0;i<((Data_Length-1)/2);i++)  { Node[i]=int(Read_Buf[2*i+8]<<8|Read_Buf[2*i+7]); }
               Node_Num = (Data_Length-1)/2;
               Node_Data_Dispose();  //处理收到的节点数据
               Serial2.write(0xFE); Serial2.write(0x00); Serial2.write(Car_Position_1); Serial2.write(0x01);  //控制码
               Serial2.write(0x00); Serial2.write(0x01); Serial2.write(0x01); Serial2.write(0xEF);  //确认数据已接收
               break;
           case 0x02:  //运行参数设置
               Car_Go = Read_Buf[6];
               Serial2.write(0xFE); Serial2.write(0x00); Serial2.write(Car_Position_1); Serial2.write(0x02);  //控制码
               Serial2.write(0x00); Serial2.write(0x01); Serial2.write(0x01); Serial2.write(0xEF);  //确认数据已接收
               break;
           case 0x03:  //状态查询
               for(char i=0;i<Data_Length;i++)  { Status_Inquiry[i]=Read_Buf[i+6];}
               Serial2.write(0xFE); Serial2.write(0x00); Serial2.write(Car_Position_1); Serial2.write(0x03);  //控制码
               Serial2.write(0x04); Serial2.write(0x00); Serial2.write(0x00); //指令码
               Serial2.write(Car_Speed); for(char i=0;i<(Data_Length-1)/2;i++) { Serial2.write(char(Node[i])); Serial2.write(char(Node[i]>>8)); }  //发送设置参数
               Serial2.write(0x01); Serial2.write(Car_Speed); Serial2.write(0x02); Serial2.write(char(Card_Data)); Serial2.write(char(Card_Data>>8)); Serial2.write(0xEF);
               break;
           case 0x04:  //中心对状态确认
               Register_Confirm = Read_Buf[6];
               break;
           case 0x05:  //中心不确认
               break;
           case 0x10:  //设置交通灯参数
               break;
           case 0x11:  //设置交通灯控制命令
               break;
        }
    }
}

/**********实时位置上报**********/
void Position_Return() 
{
    Serial2.write(0xFE); Serial2.write(0x00); Serial2.write(Car_Position_1); Serial2.write(0x05);
    Serial2.write(0x05); Serial2.write(0x00); Serial2.write(0x01); Serial2.write(Car_Speed); 
    Serial2.write(0x02); Serial2.write(char(Card_Data>>8)); Serial2.write(char(Card_Data));  Serial2.write(0xEF);
}

/********处理收到的节点数据********/
//char Go_Direction[20] = {};  //行走方向暂存数组，0为直行，1为左转，2为右转，3为停止
void Node_Data_Dispose()
{
    for(char i=0;i<(Data_Length-1)/2;i++)
    {
        Go_Direction[i] = char(Node[i]>>12);
    }
}

/********利用上位机数据进行路线规划********/
unsigned char Run_Step = 0; //执行到第几个节点计数
void Path_Planning()
{
    if(Card_Data == Node[Run_Step] && Node_Num > 0)
    {
       if(Go_Direction[Run_Step] == 0) { Car_Go = 0; }     //发车
       if(Go_Direction[Run_Step] == 1) { Turn(70,100); }   //左转
       if(Go_Direction[Run_Step] == 2) { Turn(100,100); }  //右转
       if(Go_Direction[Run_Step] == 3) { Car_Go = 1; }     //停车
       Run_Step = Run_Step+1;
       Node_Num = Node_Num-1;
    }
}

/********智能汽车注册上报********/
void Car_Confirm()
{
    for(;;)
    {
       Serial2.write(0xFE); Serial2.write(0x00); Serial2.write(Car_Position_1); Serial2.write(0x04); 
       Serial2.write(0x00); Serial2.write(0x00); Serial2.write(0xEF);//发送注册指令
       Time3s_Flag = 1; 
       do{
           serial2_Event(); 
           delay(1);
         }while(!(Register_Confirm == 1 || Time3s_Flag == 0));
         if(Register_Confirm == 1) { break; }
    }
}

void Interrupt_3s()  //12ms中断执行计时子函数,实现3s定时
{
    if(Time3s_Flag==1) 
    { 
       Time3s_Count++; 
       if(Time3s_Count>=500) { Time3s_Count = 0; Time3s_Flag = 0; } //3s计数
    }
}

/********串口数据接收函数********/
unsigned char counter = 0;
void serial2_Event() { 
  while (Serial2.available()) {
    
    Read_Buf[counter]=(unsigned char)Serial2.read();
    if(counter==0&&Read_Buf[0]!=0xFE) return;      //第0号数据不是帧头
    //Serial2.write(Read_Buf[counter]);       
    if( Read_Buf[counter] == 0xEF ) //检测帧尾
    {    
       counter=0;    //重新赋值，准备下一帧数据的接收
       Data_Receive();  //数据处理
    }
    else
        counter++; 
  }
}
