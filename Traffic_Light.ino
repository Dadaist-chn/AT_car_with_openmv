#include <Wire.h>

int Red_Stop_IO = 32;     //红灯IO PC5
int Greeen_Go_IO = 33;    //绿灯IO PC4
int Barrier_Stop_IO = 16; //障碍IO

char Light_State[3] = {1,1,1};  //信号灯状态记录。1为红灯停，2为正常走，3为前面有车停车
char Storage_Count = 0;  //信号灯状态存储计数

//**********Openmv信号口初始化**********//
void Traffic_IO_Init()
{
    pinMode(Red_Stop_IO, INPUT); 
    pinMode(Greeen_Go_IO, INPUT); 
    pinMode(Barrier_Stop_IO, INPUT); 
}

//**********Openmv数据获取**********//
void Openmv_Data_Get()  
{
  for(char i=0;i<3;i++)
  {
     if(digitalRead(Red_Stop_IO))    Light_State[i] = 1;
     if(digitalRead(Greeen_Go_IO))   Light_State[i] = 2;
     //if(digitalRead(Barrier_Stop_IO))  Light_State[i] = 3;
  }
}

//**********信号灯状态处理 **********//
char Red_Light = 0;
char Go_Straight = 0;
char Barrier_Stop = 0;
void Traffic_Light_Dispose()
{
    Openmv_Data_Get(); //红绿灯状态获取
    Red_Light = 0; Go_Straight = 0; Barrier_Stop = 0; //计数置零
    
    for(char i=0;i<3;i++)
    {
       if(Light_State[i] == 1) { Red_Light = Red_Light+1; }
       if(Light_State[i] == 2) { Go_Straight = Go_Straight+1; }
       //if(Light_State[i] == 3) { Barrier_Stop = Barrier_Stop+1; }
    }
    if(Red_Light >= 2)    {Car_Go = 1;} //停车
    if(Go_Straight >= 2)  {Car_Go = 0;}
    //if(Barrier_Stop >= 2) {Car_Go = 1;} //停车
    else  {Car_Go = 1;} //停车
}
