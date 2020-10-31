/*该文件用于管理传感器，系统启动开始初始化传感器，需要获取数据前
执行更新数据函数，然后扔来一个buffer，填充完成后返回*/

#include "board.h"
#include "api.h"
#include "sensor.h"

#define SW_SENSOR_PM25     1
#define SW_SENSOR_BATTERY  1
#define SW_SENSOR_LIGHT    1
#define SW_SENSOR_HTU21D   1

typedef union SensorSwitch
{
    uint8_t Value;
    struct SwithcBits
    {
        uint8_t pm2_5          : 1;
        uint8_t battery        : 1;
        uint8_t light          : 1;
			  uint8_t temperature    : 1;
			  uint8_t humidity       : 1;
			  uint8_t other          : 3;
    }Bits;
}SensorSwitch_t;

SensorSwitch_t SensorSwitchflag = {.Value =0}; //传感器开关标志,初始化全关闭

//挂载驱动，但不一定启用
extern Sensor pm2_5;   
extern Sensor battery;
extern Sensor light;
extern Sensor temperature;
extern Sensor humidity;

void SensorInit(void)
{ 

	#if SW_SENSOR_PM25
	  SensorSwitchflag.Bits.pm2_5=1;
    pm2_5.init(); 
	  pm2_5.configuration(5);
	  DEBUG_NORMAL( "PM2.5 device:%s | version: %s | unit %d \n ",pm2_5.name,pm2_5.version,pm2_5.unit_type);
		DelayMs(100);
  #endif
	
	#if SW_SENSOR_LIGHT
	 SensorSwitchflag.Bits.light=1;
	 light.init();
	 light.configuration(3);
	 DEBUG_NORMAL( "Light device:%s | version: %s | unit %d \n ",light.name,light.version,light.unit_type);
	 DelayMs(100);
	#endif
	
	#if SW_SENSOR_HTU21D
	
	 SensorSwitchflag.Bits.temperature=1;
	 humidity.init();
	 humidity.configuration(3);
	 DEBUG_NORMAL( "HTU21D device:%s | version: %s | unit %d \n ",humidity.name,humidity.version,humidity.unit_type);
	 DelayMs(100);
	 
	 SensorSwitchflag.Bits.humidity=1;
	 temperature.init();
	 temperature.configuration(3);
	 DEBUG_NORMAL( "HTU21D device:%s | version: %s | unit %d \n ",temperature.name,temperature.version,temperature.unit_type);
	 DelayMs(100);
	#endif

	#if SW_SENSOR_BATTERY
	 SensorSwitchflag.Bits.battery=1;
	 battery.init();
	 battery.configuration(3);
	 DEBUG_NORMAL( "Battery device:%s | version: %s | unit %d \n ",battery.name,battery.version,battery.unit_type);
	 DelayMs(100);
	#endif
}

//该函数统计出一个字节中高位的个数
int ByteCount(uint8_t byte)
{
 int num=0;
 while (byte)
 {
    num+=byte & 0x01;  //将v与00000001相与如果v的最后一位是1相与的结果是1，反之则是0。
    byte>>=1;          //右移一位。
 }
 return num;
}

//该函数根据标志位更新，也就是宏定义的传感器可以通过后续操作开关
//扔来一个buffer 填充
//修改考虑: 增加一个更新准备，使其在未调用的时候更新执行完成后不会重复更新
//这样这里便可以多个传感器同步更新，而不是一个接一个。

int SensorUpdate(char *data,int lenth)
{  
	 //如若做上面修改，则使用该变量来判断是否更新完成
	 int SensorNum = ByteCount(SensorSwitchflag.Value);
	 int pData=0;
	  DEBUG_NORMAL( "======> update sensor number :%d \n ",SensorNum);
		DelayMs(100);
		if(  SensorNum * 2  > lenth )
	  {
	    //buffer太小 
			return 0;
	  }
		


	 if(SensorSwitchflag.Bits.pm2_5)
	 {
			 while(pm2_5.update() != SENSOR_STATE_OK){};
		   uint16_t PM25Data= pm2_5.read();
			 //将高低8位交换，不然下面拷贝后是反的
			 DEBUG_NORMAL( "======> PANTENG_pm2.5 sensor :%d \n ",PM25Data);	
			 DelayMs(100);//串口输出延时。。不然进入待机了就没打印了
//			 PM25Data = ((PM25Data&0x00ff)<<8) + ((PM25Data&0xff00)>>8);
		   memcpy(data+pData, &(PM25Data), 2);
			 pData+=2;

	 }
	 
	 if(SensorSwitchflag.Bits.light)
	 {
		 while(light.update() != SENSOR_STATE_OK){};
	   uint16_t LightData = light.read();
		 LightData = (LightData&0x00ff<<8) + (LightData&0xff00>>8);
		 memcpy(data+pData, &LightData, 2);
		 pData+=2;
		 DEBUG_NORMAL( "======> light sensor :%d \n ",LightData);	
		 DelayMs(100);//串口输出延时。。不然进入待机了就没打印了
	 }
///////////////	 
	 if(SensorSwitchflag.Bits.humidity)
	 {
		 while(humidity.update() != SENSOR_STATE_OK){};
	   uint16_t HumiData = humidity.read();
		 HumiData = (HumiData&0x00ff<<8) + (HumiData&0xff00>>8);
		 memcpy(data+pData, &HumiData, 2);
		 pData+=2;
		 DEBUG_NORMAL( "======> HTU21D Humi_sensor :%d \n ",HumiData);	
		 DelayMs(100);//串口输出延时。。不然进入待机了就没打印了
	 }

	 if(SensorSwitchflag.Bits.temperature)
	 {
		 while(temperature.update() != SENSOR_STATE_OK){};
	   uint16_t TempData = temperature.read();
		 TempData = (TempData&0x00ff<<8) + (TempData&0xff00>>8);
		 memcpy(data+pData, &TempData, 2);
		 pData+=2;
		 DEBUG_NORMAL( "======> HTU21D Temp_sensor :%d \n ",(TempData/10));	
		 DelayMs(100);//串口输出延时。。不然进入待机了就没打印了
	 }

//电量检测放最后检测
		DelayMs(1000);
	 if(SensorSwitchflag.Bits.battery)
	 {
		 while(battery.update() != SENSOR_STATE_OK){};
	   uint16_t BatteryData = battery.read();
		 BatteryData = (BatteryData&0x00ff<<8) + (BatteryData&0xff00>>8);
		 memcpy(data+pData, &BatteryData, 2);
		 pData+=2;
		 DEBUG_NORMAL( "======> battery sensor :%d \n ",BatteryData);	
		 DelayMs(100);//串口输出延时。。不然进入待机了就没打印了
	 }
	 return pData;
}
