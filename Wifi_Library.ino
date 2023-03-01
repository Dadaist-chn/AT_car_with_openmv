#include <Wire.h>

void Wifi_Initialize()
{
     Serial2.begin(115200);
     //delay(2000);
     while( Serial2.read()>= 0 ) { }  //清空接收缓存区
     
     //Serial2.println("AT+CWMODE=1");  //Wifi配置
     //Serial2.println("AT+CWJAP=\"LXT2016\",\"12345678\""); //Wifi名称及密码配置
     //Serial2.println("AT+CWJAP=\"WM_ITS\",\"WM82165211.?\""); 
     
     for(;;) 
     {
          result = Serial2.find("IP");   //查询模块配置成功返回信息
          if(result)  { break; }  delay(1);
      }
     
     while( Serial2.read()>= 0 ) { }  //清空接收缓存区
     //Serial2.println("AT+CIPSTART=\"TCP\",\"192.168.1.103\",8080");
     Serial2.println("AT+CIPSTART=\"TCP\",\"192.168.1.103\",5555");  //每个人电脑IP,端口不一样，需要更改!!
     Wifi_Check();
     while( Serial2.read()>= 0 ) { }  //清空接收缓存区
     Serial2.println("AT+CIPMODE=1");
     Wifi_Check();
     while( Serial2.read()>= 0 ) { }  //清空接收缓存区
     Serial2.println("AT+CIPSEND");
     Wifi_Check();
     while( Serial2.read()>= 0 ) { }  //清空接收缓存区
  
     //Serial2.println("Wifi_Connect is OK"); 
} 

void Wifi_Check()
{
    for(;;)
    {
       result = Serial2.find("OK");
       if(result)  { break; }  delay(1);
    }
}
