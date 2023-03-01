#include <Wire.h>

int Brake_Stop1_Point[10]={}; //刹车，准备倒车入库对应卡号
int Brake_Turn_Point[10]={};  //倒车打角位置对应卡号
int Brake_Stop2_Point[10]={}; //最终停车位置对应卡号
unsigned char Stop_Number=3;  //停车点编号
char Back_Flag_1 = 0; //保证函数单次执行标志位
char Back_Flag_2 = 0;
char Back_Flag_3 = 0;

void Search_Brake_Point() //倒车入库检测函数
{
    if(Back_Flag_1 == 0)
    {
       Back_Flag_1 = 1;
       if( Card_Data == Brake_Stop1_Point[3] )  {Car_Go = 1;}  //刹车，准备倒车入库
       delay(1500);
       Car_Go = 2; //倒车
    }  
    
    if(Back_Flag_1 == 1&&Back_Flag_2 == 0)
    {
       Back_Flag_2 = 1;
       if( Card_Data == Brake_Turn_Point[3] )  {Turn(105,63);}  //刹车，准备倒车入库
    }
    
    if(Back_Flag_1 == 1&&Back_Flag_2 == 1&&Back_Flag_3 == 0)
    {
       Back_Flag_3 = 1;
       if( Card_Data == Brake_Stop2_Point[3] )  {Car_Go = 1;}  //刹车
    }
    
    if(Back_Flag_1 == 1&&Back_Flag_2 == 1&&Back_Flag_3 == 1)   //标志位置零
    {
       Back_Flag_1 = 0; Back_Flag_2 = 0; Back_Flag_3 = 0; 
    }
}
