/**
  ******************************************************************************
  * @file    yishan_PM2_5.h
  * @author  LPV6-lissettecarlr
  * @version V0.1(stm32L151)
  * @date    16/11/2017(create)  
  * @brief   
						每秒发送一次，波特率9600 一个包长度为32个字节
						请求数据：
						33 3E 00 0C A4 00 00 00 00 00 00 00 00 00 01 21
						设定输出间隔（5s）
						33 3E 00 0C A3 00 00 05 00 00 00 00 00 00 01 25 
						定时输出  开/ / 关
						33 3E 00 0C A2 00 00 01 00 00 00 00 00 00 01 20 
						33 3E 00 0C A2 00 00 00 00 00 00 00 00 00 01 1F 
						传感器开关功能
						33 3E 00 0C A1 00 00 00 00 00 00 00 00 00 01 1E  关
						33 3E 00 0C A1 00 00 01 00 00 00 00 00 00 01 1F
            
  ******************************************************************************
*/

#ifndef __YISHAN_PM2_5_H__
#define __YISHAN_PM2_5_H__

#include "sensor.h"

void        yishan_pm2_5_init(void);
void        yishan_pm2_5_configuration(int time);
uint16_t    yishan_pm2_5_read(void);
SensorState yishan_pm2_5_update(void);



#endif
