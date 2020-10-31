#include "yishan_pm25.h"
#include "serial_board.h"
#include "delay.h"
#include "systick.h"

static uint8_t order_yishan[16]={0x33,0x3E,0x00,0x0C,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1D};
static uint8_t order_ON[16] =   {0x33,0x3E,0x00,0x0C,0xA1,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1F};
static uint8_t order_OFF[16] = {0x33,0x3E,0x00,0x0C,0xA1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1E};
static uint8_t OrderGet[16]={0x33,0x3E,0x00,0x0C,0xA4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x21};
static uint8_t OrderTimeClose[16]={0x33,0x3E,0x00,0x0C,0xA2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1F};	
static uint8_t OrderTimeOpen[16]= {0x33,0x3E,0x00,0x0C,0xA2,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x20};
static uint8_t ordercycle5[16]  = {0x33,0x3E,0x00,0x0C,0xA3,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x25};

#define ORDER_YISHAN             1
#define ORDER_ON                 2
#define ORDER_OFF                3
#define ORDER_GET                4
#define ORDER_TIME_CLOSE         5
#define ORDER_TIME_OPEN          6
#define ORDER_CYCLE_5S           7

static uint16_t data=0;
static int timeout=SENSOR_TIMEOUT_DEFAULT;

Sensor pm2_5 =
{
    "yishan_pm2_5",
	  "0.1v",
	  SENSOR_PM2_5,
	  UG_M3,
	  yishan_pm2_5_init,
	  yishan_pm2_5_configuration,
	  yishan_pm2_5_read,
	  yishan_pm2_5_update,
};



static int SendOrder(char type)
{
  switch(type)
	{
	  case ORDER_YISHAN:
			SerialBoardSendData(order_yishan,16);
			return ORDER_YISHAN;
		
		case ORDER_OFF:
			SerialBoardSendData(order_OFF,16);
			return ORDER_OFF;
		
		case ORDER_ON:
			SerialBoardSendData(order_ON,16);
			return ORDER_ON;
		
		case ORDER_GET:
			SerialBoardSendData(OrderGet,16);
			return ORDER_GET;
		
		case ORDER_TIME_CLOSE:
			SerialBoardSendData(OrderTimeClose,16);
		  return ORDER_TIME_CLOSE;
		
		case ORDER_TIME_OPEN:
			SerialBoardSendData(OrderTimeOpen,16);
		  return ORDER_TIME_OPEN;
		
		case ORDER_CYCLE_5S:
			SerialBoardSendData(ordercycle5,16);
		  return ORDER_CYCLE_5S;
		
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

void yishan_pm2_5_init()
{
   data = SENSOR_DEFAULT_TIMEOUT;
	 SendOrder(ORDER_YISHAN);
	 DelayMs(100); //看传感器而定，这个传感器发送快了就GG
	 SendOrder(ORDER_TIME_CLOSE);
	 DelayMs(100);
	 SendOrder(ORDER_OFF);
	 DelayMs(100);
}
	
void yishan_pm2_5_configuration(int time)
{
	 // PM25 Not allowed to set a short time
	 if(time > SENSOR_TIMEOUT_DEFAULT)
       timeout = time;
}

uint16_t yishan_pm2_5_read()
{
   return data;
}
	
static SensorState sensor_update_flag=SENSOR_STATE_READY;

SensorState yishan_pm2_5_update()
{
	 static double record;
	 static int Lenth;
	 switch(sensor_update_flag)
	 {
	   case SENSOR_STATE_READY:
			 data = SENSOR_DEFAULT_TIMEOUT;
		   record = GetNowTime();//记录开始更新时间
		   ClearSerialBoardBuffer();
		   sensor_update_flag = SENSOR_STATE_BUSY;
		   SendOrder(ORDER_ON);
			 DelayMs(100);	
			 break;
		 case SENSOR_STATE_BUSY:
			  if(GetNowTime() - record >= timeout/2) //先使传感器运行一段时间
				{
		        SendOrder(ORDER_GET);
					  sensor_update_flag = SENSOR_STATE_OTHER;
				}
			 break;
		 case SENSOR_STATE_OTHER: 
			  if(GetNowTime() - record >=timeout)//超时
				{
					  data = SENSOR_DEFAULT_TIMEOUT;
				    sensor_update_flag = SENSOR_STATE_OK;
					  SendOrder(ORDER_OFF);
					  DelayMs(100);			  
				}
				Lenth = GetSerialBoardReciveDataSize();
				//这里之后需要一个工具函数（从一堆数据中提取正确的一帧数据）
				if(Lenth >=32)
				{
					 char RcvData[32];
				   GetSerialBoardReciveData(RcvData,32); 
           if(mCheck( (uint8_t *)RcvData , 32))//和校验
					 {
						  data =((uint16_t)RcvData[6])*256+RcvData[7];
					    sensor_update_flag = SENSOR_STATE_OK;
					 }
					 else
					 {
					    data = SENSOR_DEFAULT_CRC_ERROR;
						  sensor_update_flag = SENSOR_STATE_OK;
					 }
					 SendOrder(ORDER_OFF);
					 DelayMs(100);
				}
				
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
 
