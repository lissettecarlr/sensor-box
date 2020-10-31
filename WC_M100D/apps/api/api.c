/*
该套程序用于传感器数据采集，并且将数据通过M100D模组上发。
流程: 休眠-->传感器更新-->唤醒通信模组-->数据上传（数据接收）-->休眠
*/

#include "board.h"
#include "api.h"
#include "communication.h"
#include "serial3_board.h"

#define APP_TX_DUTYCYCLE_RND                        1000
#define VERSION 																		2

char SendBuffer[255];
int SendBufferLenth=0;
extern communication WirelessModule;
static TimerEvent_t TxNextPacketTimer;
static TimerEvent_t WirelessQueryStateTimer;
	
enum eDevicState
{
	 DEVICE_STATE_INIT, 
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP
}DeviceState;


void OnTxNextPacketTimerEvent()
{
	  BoardEnableIrq();
	  SendBufferLenth=SensorUpdate(SendBuffer,100);

		for(int i = SendBufferLenth;i>= 0 ;i-- )
		{
				SendBuffer[i]=SendBuffer[i-1];
		}
				SendBuffer[0]=VERSION;
			SendBufferLenth	+= 1;

for(int j =0;j<11;j++){
		DEBUG_NORMAL("SendBuffer = %d\n",SendBuffer[j]);
}
			
	  DeviceState = DEVICE_STATE_SEND;

	  BoardDisableIrq();
}


void WirelessQueryStateEvent()
{
  //对模组发送状态查询包	
  qurey_wireless_state_pck();
  DeviceState =DEVICE_STATE_JOIN;
}

static uint32_t UploadCycle= 60000;//初始化上传时间 60s

void set_upload_cycle(int cycle) //单位s
{
    UploadCycle = cycle * 1000;
}

int main( void )
{
    uint32_t TxDutyCycleTime;
    static uint8_t busy_timer=0;	
	  uint32_t QueryWirelessStateCycle = 180000;
    double Record =0;
	
    BoardInitMcu();
    DeviceState = DEVICE_STATE_INIT;
		  
	  RCC_ClocksTypeDef get_rcc_clock;
    RCC_GetClocksFreq(&get_rcc_clock);
	  /* 获取系统时钟 */
	 
    while( 1 )
    {
      switch( DeviceState )
      {
		  case DEVICE_STATE_INIT:
			  //通信初始化
		    M100d_Init(set_upload_cycle); 
			  SensorInit();
			  TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent, "tx next packet" );
			  TimerInit( &WirelessQueryStateTimer, WirelessQueryStateEvent, "Reconnec Join" );
		 
        DeviceState = DEVICE_STATE_JOIN;
		   // DEBUG_NORMAL("DEBUG--------->system init ok\n");
			  //DelayMs(100);
			  break;
						
				case DEVICE_STATE_JOIN: 

				   Record = GetNowTime();
				   while(1)
				   {
							if(get_network_state() ==1)
							{


//**************************************************************//
				//入网成功马上跟新传感器值
					DelayMs(100); 
					m100d_wake(); //唤醒传感器
					if(SendBufferLenth > 0)
					{
					  wireless_send_data((uint8_t *)SendBuffer,SendBufferLenth); 
						DelayMs(500);
					}
					else
					  DEBUG_NORMAL("SendBufferLenth = 0\n");	
					SysDelayMs(5000);//该延时用于等待模组返回数据，顺带接收下行数据
//**************************************************************//

									//连接成功，设置上传周期	
								DeviceState = DEVICE_STATE_CYCLE;
					//DEBUG_NORMAL("DEBUG--------->join ok\n");
				//	DelayMs(100);
								break;
							}
							if(ClockTool(&Record,60) )
							{				    
								//如果一段时间还连接不上，则进入休眠，依靠状态查询来判断模组是否入网								 							 
							//	DEBUG_NORMAL("join fail ! go to sleep,After a period of time to try\n");
								//DelayMs(100);
								TimerSetValue( &WirelessQueryStateTimer, QueryWirelessStateCycle );	   
								TimerStart( &WirelessQueryStateTimer );
								DeviceState =DEVICE_STATE_SLEEP;
								break;
							}
						}
						break;
            case DEVICE_STATE_SEND:			
					//更新传感器的值，将其发送出去，等待返回信息
					//DEBUG_NORMAL("send : %d type\n",SendBufferLenth);		
					
					/*模组出现问题时的处理方式，由于是classA，所以下行比较麻烦*/
          if(get_network_state() == 3)//处于设备忙
					{
						busy_timer++;
						if(busy_timer >= 10)//如果10次上传都处于设备忙，则复位设备
						{
						    busy_timer=0;
							  set_network_state(0);
							  DeviceState = DEVICE_STATE_JOIN;
							  m100d_rst();
							  SysDelayMs(2000);//等待模组复位时间
							  break;
						}
					}
          else
					{
					   busy_timer=0;
					}					
					DelayMs(100); 
					m100d_wake(); //唤醒传感器
					if(SendBufferLenth > 0)
					{
					  wireless_send_data((uint8_t *)SendBuffer,SendBufferLenth); 
						DelayMs(500);
					}
					else
					  DEBUG_NORMAL("SendBufferLenth = 0\n");	
					SysDelayMs(5000);//该延时用于等待模组返回数据，顺带接收下行数据
					//WirelessModule.close();//模组进入睡眠模式
					DeviceState = DEVICE_STATE_CYCLE;
					break;
                
        case DEVICE_STATE_CYCLE:
          // 定时器重置
					TxDutyCycleTime = UploadCycle ;
					TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
					TimerStart( &TxNextPacketTimer );
					//DEBUG_NORMAL("cycle ok! \n");
					//DelayMs(100);
					DeviceState = DEVICE_STATE_SLEEP;
					break;
            
        case DEVICE_STATE_SLEEP:
						//McuEnterLowPowerMode();
            break;
            
        default:
            break;
        }        
        McuEnterLowPowerMode();
    }
}
