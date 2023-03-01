#include <Wire.h>

#define MOTOR1 13   // 电机INR
#define MOTOR2 12   // 电机INF

//********小车参数修改处 *********//
float Servo_Kp = 5;  //PID参数设置
float Servo_Kd = 9;

unsigned char Car_Number = 1; //使用的小车编号,换车时修改此处,“1”对应1号车
unsigned char Server_mid_value[25]={85,90,85};  //舵机中值调整数组，分别对应各辆车

//*********电机初始化*********//
void Motor_Init(){
   pinMode(MOTOR1, OUTPUT);  pinMode(MOTOR2, OUTPUT);
}

//**********原始值获取**********//
uint16_t adc_max[4] = {800, 800, 1000, 1000};       //获取的最大值
uint16_t adc_get[4] = {0};                          //原始值
uint16_t adc_filter[4] = {0,1,2,3};  
void run_get()
{
  adc_get[0] =  analogRead(Ind1);
  adc_get[1] =  analogRead(Ind2);
  adc_get[2] =  analogRead(Ind3);
  adc_get[3] =  analogRead(Ind4);
}

//**********滤波**********//
#define N 8                                  //滑动滤波队列大小
uint8_t N_i;                                  //滑动值
uint16_t temp[N][4] = {0};                    //队列数组

void run_filter(void)
{
  //滑动滤波
  uint16_t sum[4] = {0, 0, 0, 0};             //累和归零
  uint8_t j;                                  //循环变量
  //原始数据获取 ad每5ms读取一次结果
  for (j = 0; j < 4; j++)
  {
    temp[N_i][j] = adc_get[j];
  }
  N_i++;
  if (N_i == N)
    N_i = 0;
  //累和
  for (j = 0; j < N; j++)
  {
    sum[0] += temp[j][0];
    sum[1] += temp[j][1];
    sum[2] += temp[j][2];
    sum[3] += temp[j][3];
  }
  for (j = 0; j < 4; j++)
  {
    adc_filter[j] = sum[j] / 8;               //求平均
  }
}

//**********归一化**********//
uint16_t adc_guiyi[4];              //归一化值

void run_guiyi()
{
  float guiyi[4] = {0, 0, 0, 0};
  uint8_t i;
  for (i = 0; i < 4; i++)
  {
    guiyi[i] = (float)adc_filter[i] / adc_max[i];
    if (guiyi[i] <= 0.0f)
      guiyi[i] = 0.01f;
    if (guiyi[i] >= 1.0f)
      guiyi[i] = 1.0f;
    adc_guiyi[i] = (uint8_t)(guiyi[i] * 100);
    if (adc_guiyi[i] < 1u)
      adc_guiyi[i] = 1u;
  }
 }

//**********计算偏差并赋值给舵机**********//
float car_error = 0;         //当前偏差,偏差有正有负
float car_error_last = 0;    //上次偏差
float servo_out = 0;         //float
float real = 0; 

void run_error()
{                                                                                                                                                         
  float sqrt0 = 0.0, sqrt1 = 0.0;
  sqrt0 = sqrt(adc_guiyi[2]);
  sqrt1 = sqrt(adc_guiyi[3]);
  //计算偏差
  car_error = (float)( (sqrt0 - sqrt1) / (adc_guiyi[0] + adc_guiyi[1]) );
  car_error = (float)(car_error * 100u);
  //PD
  servo_out = Servo_Kp * car_error + Servo_Kd * (car_error - car_error_last);
  //更新偏差
  car_error_last = car_error;

  //限幅(70-110)
  if (servo_out > 40)  { servo_out = 40; }
  if (servo_out < -40) { servo_out = -40; }
  real = servo_out / 2 + Server_mid_value[Car_Number-1];  myservo.write(real);  //小车舵机中值调整.变小左转,变大右转
}

//**********Wifi发回电感值**********//
void ADC_Value_Send()
{
    Serial2.print(adc_filter[2]);
    Serial2.print("   ");
    Serial2.println(adc_filter[3]);
}

//**********电机调试函数**********//
void Motor_Turn_Test()
{
    analogWrite(MOTOR1, 240); digitalWrite(MOTOR2,LOW);  //正确为正转
}

