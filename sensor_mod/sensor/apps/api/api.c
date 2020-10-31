//串口1打印115200，串口2与PM2.5通信，串口3和通讯模组沟通，传感器自更新30s一次
//晶振12M
#include "board.h"
#include "api.h"
#include "serial3_board.h"
#include "six_box_pto.h"

#define SENSOR_UPDATE_CYCLE                         30000
#define VERSION 																		1

char SendBuffer[255];
int SendBufferLenth=0;


static TimerEvent_t sensorUpdateTimer;


void ledInit()
{

	//模组复位引脚初始化
    GPIO_InitTypeDef  GPIO_InitStructure;
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType= GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	  GPIO_SetBits(GPIOB,GPIO_Pin_9);
	
}

void ledFlash()
{
    GPIO_SetBits(GPIOB,GPIO_Pin_9);
	  DelayMs( 500 );
	  GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	
}

void OnSensorUpdateTimerEvent()
{
	  BoardEnableIrq();
	  SendBufferLenth=SensorUpdate(SendBuffer,100);

		for(int i = SendBufferLenth;i>= 0 ;i-- )
		{
				SendBuffer[i]=SendBuffer[i-1];
		}
		SendBuffer[0]=VERSION;
		SendBufferLenth	+= 1;
    DEBUG_NORMAL("sensor datasize = %d\n",SendBufferLenth);
		DEBUG_NORMAL(" [ ");
for(int j =0;j<SendBufferLenth;j++){
		DEBUG_NORMAL("%02X  ",SendBuffer[j]);
}	
    DEBUG_NORMAL(" ]\n");
	  BoardDisableIrq();
    TimerStart( &sensorUpdateTimer );
}


int main( void )
{
	  uint8_t RcvData[255];
		uint16_t frameSize=0;
	  uint8_t Data[10];
	  uint8_t frame[50];
    BoardInitMcu();
	  SensorInit();
	  SB_init();
	  ledInit();
	  TimerInit( &sensorUpdateTimer, OnSensorUpdateTimerEvent, "sensor updata" );
		TimerSetValue( &sensorUpdateTimer, SENSOR_UPDATE_CYCLE );	   
		TimerStart( &sensorUpdateTimer );
	  ledFlash();
	  while(1)
		{
			 //Serial3BoardSendData("123",strlen("123"));
		   //每30s采集一次传感器值
			 //不断检查是否有数据请求
			 if(isRcvData())
			 {
			    int size = GetSerial3BoardReciveDataSize();
			    DEBUG_NORMAL("size %d\n",size);
				  if(size<9)
					{
					    ClearSerial3BoardBuffer();
						  continue;
					}
			    GetSerial3BoardReciveData(RcvData,9);
			    ClearSerial3BoardBuffer();
				  //拆解数据包
					uint8_t temp = SB_disassemble(RcvData,9,Data,&frameSize) ;
					DEBUG_NORMAL( "######SB_disassemble %d\n", temp); 
				  if(temp == SB_TYPE_GET)//应答
					{
						 
						 SB_prepare_frame(SB_TYPE_DATA,SendBuffer,SendBufferLenth,frame,&frameSize);
					   Serial3BoardSendData(frame,frameSize);
						 ledFlash();
					}
			 }
		}   
      //  McuEnterLowPowerMode();
    
}
