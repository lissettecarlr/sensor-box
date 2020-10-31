#include "panteng_pm25.h"
#include "serial_board.h"
#include "delay.h"
#include "systick.h"

static uint8_t OrderNormal[7] = {0x42,0x4d,0xe4,0x00,0x01,0x01,0x74};//待机模式——开/正常模式
static uint8_t OrderSleep[7] = {0x42,0x4d,0xe4,0x00,0x00,0x01,0x73};//待机模式——关/关闭传感器
static uint8_t OrderGet[7]={0x42,0x4d,0xe2,0x00,0x00,0x01,0x71};
//状态切换——主动式，类似待机模式开，但风扇开启
static uint8_t OrderInitiative[7]={0x42,0x4d,0xe1,0x00,0x01,0x01,0x71};
static uint8_t OrderPassivity[7]={0x42,0x4d,0xe1,0x00,0x00,0x01,0x70};//状态切换——被动式，风扇开启	

//42 4D E4 00 01 01 74 
//42 4D E1 00 00 01 70 
//42 4D E2 00 00 01 71 

#define ORDER_NORMAL             1
#define ORDER_SLEEP              2
#define ORDER_GET                3
#define ORDER_INITIATIVE         4
#define ORDER_PASSIVITY          5

static uint16_t data=0;
static int timeout=SENSOR_TIMEOUT_DEFAULT;

Sensor pm2_5 =
{
    "PANTENG_pm2_5",
	  "0.1v",
	  SENSOR_PM2_5,
	  UG_M3,
	  PANTENG_pm2_5_init,
	  PANTENG_pm2_5_configuration,
	  PANTENG_pm2_5_read,
	  PANTENG_pm2_5_update,
};



static int SendOrder(char type)
{
  switch(type)
	{
	  case ORDER_NORMAL:
			SerialBoardSendData(OrderNormal,7);
			return ORDER_NORMAL;
		
		case ORDER_SLEEP:
			SerialBoardSendData(OrderSleep,7);
			return ORDER_SLEEP;
		
		case ORDER_GET:
			SerialBoardSendData(OrderGet,7);
			return ORDER_GET;
		
		case ORDER_INITIATIVE:
			SerialBoardSendData(OrderInitiative,7);
			return ORDER_INITIATIVE;
		
		case ORDER_PASSIVITY:
			SerialBoardSendData(OrderPassivity,7);
		  return ORDER_PASSIVITY;
		
		default:
			return 0;
	}
}


static bool mCheck(uint8_t *Data,int Lenth)
{
	uint16_t CheckSum = (((uint16_t)Data[Lenth-2])<<8)+Data[Lenth-1];
	int temp=0;
	for(int i=0;i<Lenth-3;i++)
	{
		temp+=Data[i];
	}
	if(temp == CheckSum)
		return true;
	else
		return false;
}

void PANTENG_pm2_5_init()
{
   data = SENSOR_DEFAULT_TIMEOUT;
//	 SendOrder(ORDER_NORMAL);
	 DelayMs(100);
	 SendOrder(ORDER_PASSIVITY);
	 DelayMs(300); //看传感器而定，这个传感器发送快了就GG
	 SendOrder(ORDER_SLEEP);
	 DelayMs(300);
}
	
void PANTENG_pm2_5_configuration(int time)
{
	 // PM25 Not allowed to set a short time
	 if(time > SENSOR_TIMEOUT_DEFAULT)
       timeout = time;
}

uint16_t PANTENG_pm2_5_read()
{
   return data;
}
	
static SensorState sensor_update_flag=SENSOR_STATE_READY;

SensorState PANTENG_pm2_5_update()
{
	 static double record =0 ;
	 int Lenth =0 ;
	
	 switch(sensor_update_flag)
	 {
	   case SENSOR_STATE_READY:
			 data = SENSOR_DEFAULT_TIMEOUT;
		   record = GetNowTime();//记录开始更新时间
		   ClearSerialBoardBuffer();
		   sensor_update_flag = SENSOR_STATE_BUSY;
		 	 SendOrder(ORDER_NORMAL);
			 DelayMs(500);
		 	 SendOrder(ORDER_PASSIVITY);
			 DelayMs(500);
		 break;
		 
		 case SENSOR_STATE_BUSY:

			  if(GetNowTime() - record >= timeout) //先使传感器运行一段时间
				{
						DelayMs(100);
		        SendOrder(ORDER_GET);
						DelayMs(100);
//					SendOrder(ORDER_SLEEP);
//					DelayMs(100);
					  record = GetNowTime();//记录开始更新时间
					  sensor_update_flag = SENSOR_STATE_OTHER;
				}
		 break;
		 
		 case SENSOR_STATE_OTHER:
			  Lenth = GetSerialBoardReciveDataSize();
		    if(Lenth >=24)
				{
					 char RcvData[24];
					
				   GetSerialBoardReciveData(RcvData,24);
					 SendOrder(ORDER_SLEEP);
					 DelayMs(100);
					
           if(mCheck( (uint8_t *)RcvData , 24) )//和校验
					 {
						  data =((uint16_t)RcvData[6])*256+RcvData[7];
					    sensor_update_flag = SENSOR_STATE_OK;
					 }
					 else
					 {
					    data = SENSOR_DEFAULT_CRC_ERROR;
						  sensor_update_flag = SENSOR_STATE_OK;
					 }
					 
  					  SendOrder(ORDER_SLEEP);
		 			    DelayMs(100);
					    ClearSerialBoardBuffer(); 
					    sensor_update_flag = SENSOR_STATE_OK;
              break;					 
				}
				
			  if(GetNowTime() - record >=timeout)//超时
				{
					  data = SENSOR_DEFAULT_TIMEOUT;
				    sensor_update_flag = SENSOR_STATE_OK;
  					SendOrder(ORDER_SLEEP);
					  DelayMs(100);		
					  ClearSerialBoardBuffer();
					  sensor_update_flag = SENSOR_STATE_OK;
				}				
//				char RcvData1[24];
//				GetSerialBoardReciveData(RcvData1,24);

			    
		 break;
		 
		 case SENSOR_STATE_OK:
			  sensor_update_flag=SENSOR_STATE_READY;
		 
		 break;
	 }
	 
	 //由于增加了一个状态，不能直接返回
	 if(sensor_update_flag == SENSOR_STATE_OTHER)
		 return SENSOR_STATE_BUSY;
	 else
     return sensor_update_flag;
}
 
//环境检测设备
//外壳采用叠状设计，放置雨水渗入，采集环境的温湿度、光照、PM2,5数据，通过FSK或者lora周期上报。