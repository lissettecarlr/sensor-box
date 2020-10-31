#include "panten_PMS3003ST.h"
#include "serial_board.h"
#include "delay.h"
#include "debug.h"
//5种发送指令
//1: 获取数据
//2：切换到主动发送模式
//3：切换到被动发送模式
//4：待机模式
//5；正常模式

static unsigned char OrderGet[]=         {0x42,0x4D,0xE2,0x00,0x00,0x01,0x71};
static unsigned char OrderPassivity[]=   {0x42,0x4D,0xE1,0x00,0x00,0x01,0x70}; //被动
static unsigned char OrderInitiative[] = {0x42,0x4D,0xE1,0x00,0x01,0x01,0x71};
static unsigned char OrderSleep[]=       {0x42,0x4D,0xE4,0x00,0x00,0x01,0x73};
static unsigned char OrderNormal[]=      {0x42,0x4D,0xE1,0x00,0x01,0x01,0x74};

#define DATA_STATE_NORMAL  0
#define DATA_STATE_CRC     1
#define DATA_STATE_LOST    2


static char LowPower=0;    //低功耗标识
static char DataState=0;   //数据状态标识，0标识正常，1表示crc，2表示丢失
static char Data[2];
static char Data_error_crc[8] = {0xff,0x91,0xff,0x91,0xff,0x91,0xff,0x91};
static char Data_error_lost[8] = {0xff,0x92,0xff,0x92,0xff,0x92,0xff,0x92};


#define ORDER_GET             1
#define ORDER_MODE_INITIATIVE 2
#define ORDER_MODE_PASSIVITY  3
#define ORDER_SLEEP           4
#define ORDER_NORMAL          5


Sensor panten_pm2_5 =
{
    "panteng_pms_3003",
	  "0.1v",
	  SENSOR_PM2_5,
	  UG_M3,
	  panten_PMS3003ST_init,
	  panten_PMS3003ST_configuration,
	  panten_PMS3003ST_read,
	  panten_PMS3003ST_update,
};

static uint16_t data=0;
static int timeout=SENSOR_TIMEOUT_DEFAULT;

static int SendOrder(char type)
{
  switch(type)
	{
	  case ORDER_GET:
			SerialBoardSendData(OrderGet,7);
			return ORDER_GET;
		case ORDER_MODE_INITIATIVE:
			SerialBoardSendData(OrderInitiative,7);
			return ORDER_MODE_INITIATIVE;
		case ORDER_MODE_PASSIVITY:
			SerialBoardSendData(OrderPassivity,7);
			return ORDER_MODE_PASSIVITY;
		case ORDER_SLEEP:
			SerialBoardSendData(OrderSleep,7);
			return ORDER_SLEEP;
		case ORDER_NORMAL:
			SerialBoardSendData(OrderNormal,7);
		  return ORDER_NORMAL;
		default:
			return 0;
	}
}

//sw = 1 : Open low power
void panten_PMS3003ST_init(void)
{
	// DEBUG_NORMAL("=====> Sensor-PMS3003ST-init\n");
//   SendOrder(ORDER_MODE_INITIATIVE);//Initiative
//	 DelayMs(100);
//	 if(sw)
//	  SendOrder(ORDER_SLEEP);//Sleep
//	 else
//		SendOrder(ORDER_NORMAL);
//	 LowPower=sw;
	 
	 data = SENSOR_DEFAULT_TIMEOUT;
	 SendOrder(ORDER_MODE_INITIATIVE);
	 DelayMs(100); //看传感器而定，这个传感器发送快了就GG
	 
}

static char check(char *data,int len)
{
	unsigned short sum=0;
  for(int i=0;i<len-2;i++)
	 sum+=data[i];
	unsigned short check = (data[len-2]<<8) + data[len-1];
	if(sum == check)
		return 1;
	else
		return 0;
}

void panten_PMS3003ST_configuration(int time)
{
	 // PM25 Not allowed to set a short time
	 if(time > SENSOR_TIMEOUT_DEFAULT)
       timeout = time;
}

uint16_t panten_PMS3003ST_read(void)
{
	return data;
}

//执行完毕后数据有三种形式
//正常、校验错误、无数据
SensorState panten_PMS3003ST_update(void)
{
	 DEBUG_NORMAL("=====> Sensor-PMS3003ST-update-start\n");
	 if(LowPower) //WAKE UP
	 {
		 SendOrder(ORDER_NORMAL);
		 //由于唤醒后采 集数据需要一定时间，于是开启一个定时器，该定时器回调本函数。
     Delay(6);
	 }
	 ClearSerialBoardBuffer();
   SendOrder(ORDER_GET);
	 Delay(1);//等待数据返回
	 int Lenth = GetSerialBoardReciveDataSize();
	 if(Lenth == 40)
	 {
		   char RcvData[40];
		   GetSerialBoardReciveData(RcvData,40);
		   // CRC
		   if(check(RcvData,40))
			 {
			    DataState=DATA_STATE_NORMAL;
				  Data[0] = RcvData[12];
				  Data[1] = RcvData[13];//PM2.5

			 }
			 else
			 {
			    DataState=DATA_STATE_CRC;
			 }
	 }
	 else
	 {
			 // 无数据
		  DataState=DATA_STATE_LOST;
	 }
	 SendOrder(ORDER_SLEEP);
	 //DEBUG_NORMAL("=====> Sensor-PMS3003ST-update-over : %d \n",DataState);

}
