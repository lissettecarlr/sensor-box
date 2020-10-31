#include "htu21d.h"
#include "i2c.h"
#include "systick.h"

Sensor temperature =
{
    "hut21d_temperature",
	  "0.1v",
	  SENSOR_TEMPERATURE,
	  DEGREE_CENTIGRADE, 
	  htu21d_init,
	  htu21d_configuration,
	  htu21d_read_temp,
	  htu21d_update_temp,
};

Sensor humidity =
{
    "hut21d_humidity",
	  "0.1v",
	  SENSOR_HUMIDIYT,
	  PERCENTAGE,
	  htu21d_init,
	  htu21d_configuration,
	  htu21d_read_humi,
	  htu21d_update_humi,
};

static uint8_t htu21d_init_flag=0; ////避免温度和湿度的两次初始化
//static uint8_t htu21d_state=0; // 1: init error  |  0 :normal
static uint16_t temp=0; // 保存值 = 实际值*10 例如12.1° = 121
static uint16_t humi=0; 

#define   HTU21D_ADR_W          0x80
#define   HTU21D_ADR_R          0x81  
#define   HTU21D_STATUS_MASK  	0xfc
#define   HTU21D_TEMP           0xE3 
#define   HTU21D_HUMI           0xE5 
#define   HTU21D_SOFT_RESET     0xFE 

uint8_t HTU21D_SoftReset(void)
{
	uint8_t err=0;
	IIC_Start();
  IIC_Send_Byte(HTU21D_ADR_W);
	err = IIC_Wait_Ack();
	IIC_Send_Byte(HTU21D_SOFT_RESET);
	err = IIC_Wait_Ack();
	IIC_Stop();
	return err;
}

void htu21d_init(void)
{
   IIC_Init();
  // htu21d_state = 
	 HTU21D_SoftReset(); 
	 htu21d_init_flag=1;
}

void htu21d_configuration(int time)
{
}

uint16_t  htu21d_read_temp(void)
{
   return temp;
}

uint16_t htu21d_read_humi(void)
{
  return humi;
}

SensorState  htu21d_update_temp(void)
{
    uint8_t ack, tmp1, tmp2;
    uint16_t i=0;
	  float temp_f=0;
	  if(!htu21d_init_flag)//初始化失败
		{
			//重新尝试初始化。尝试时间 为超时时间
		  temp = SENSOR_DEFAULT_TIMEOUT;
			return SENSOR_STATE_OK; 
		}
		IIC_Start();
		IIC_Send_Byte(HTU21D_ADR_W);
		ack = IIC_Wait_Ack();
		IIC_Send_Byte(HTU21D_TEMP);
		ack = IIC_Wait_Ack();
	  do {
        SysDelayMs(100);               
        IIC_Start();				
        IIC_Send_Byte(HTU21D_ADR_R);	
			  i++;
			  ack = IIC_Wait_Ack();
			  if(i==10)break;
    } while(ack!=0);
		tmp1 = IIC_Read_Byte(1);
    tmp2 = IIC_Read_Byte(1);
		IIC_Read_Byte(0);
    IIC_Stop();
		
		uint16_t ret = (tmp1 << 8) | (tmp2 & HTU21D_STATUS_MASK);	
    temp_f = ret*175.72/65536-46.85;		
		temp = (uint16_t)(temp_f*10.0);
		
		return SENSOR_STATE_OK; 
}


SensorState htu21d_update_humi(void)
{
    uint8_t ack, tmp1, tmp2;
		uint16_t i=0;
    if(!htu21d_init_flag)
		{
		
		  humi = SENSOR_DEFAULT_TIMEOUT;
			return SENSOR_STATE_OK; 
		}
		
    IIC_Start();				
    IIC_Send_Byte(HTU21D_ADR_W);
		ack = IIC_Wait_Ack();	
		IIC_Send_Byte(HTU21D_HUMI);
		ack = IIC_Wait_Ack();    
    do {
        SysDelayMs(100);               
        IIC_Start();			
        IIC_Send_Byte(HTU21D_ADR_R);	
			  i++;
			  ack = IIC_Wait_Ack();
			  if(i==10)break;
    } while(ack!=0);
  
    tmp1 = IIC_Read_Byte(1);
    tmp2 = IIC_Read_Byte(1);
    IIC_Read_Byte(0);
    IIC_Stop();
 
		uint16_t	ret = (tmp1 << 8) | (tmp2 & HTU21D_STATUS_MASK);		
	  humi =(uint16_t)(ret*125/65536-6);	
		
    return SENSOR_STATE_OK; 
}
