/*该驱动仅仅是用于空气小六硬件将串口1作为模拟IIC口来对接土壤传感器而编写的*/

#include "sth20.h"

/*模拟IIC*/

#include "stm32l1xx.h"
#include "board.h"
#include "systick.h"
#include "stm32l1xx_gpio.h"
 
// 7 -SAD  ->PA9

#define IIC2_SCL_H      GPIO_SetBits(GPIOA,GPIO_Pin_10) 
#define IIC2_SCL_L      GPIO_ResetBits(GPIOA,GPIO_Pin_10)

#define IIC2_SDA_H      GPIO_SetBits(GPIOA,GPIO_Pin_9)     		
#define IIC2_SDA_L      GPIO_ResetBits(GPIOA,GPIO_Pin_9)

#define IIC2_READ_SDA   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9)


void IIC2_Delay(void)
{
	SysDelayUs(4); 
}

void IIC2_Init(void)
{					     
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA ,ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType= GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);		 						
}

void IIC2_SET_SDA_IN()
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
 	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void IIC2_SET_SDA_OUT()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType= GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void IIC2_Start(void)
{
	IIC2_SET_SDA_OUT();    
	IIC2_SDA_H;	  	  
	IIC2_SCL_H;
	SysDelayUs(4);
 	IIC2_SDA_L;
	SysDelayUs(4);
	IIC2_SCL_L;
	SysDelayUs(4);
}	 

void IIC2_Stop(void)
{
	IIC2_SET_SDA_OUT();
	IIC2_SCL_L;
	IIC2_SDA_L;
 	SysDelayUs(4);
	IIC2_SCL_H;
	SysDelayUs(1);
	IIC2_SDA_H;
	SysDelayUs(4);						   	
}


uint8_t IIC2_Wait_Ack(void)
{
	uint16_t ucErrTime=0;
	IIC2_SET_SDA_IN();       
	IIC2_SDA_H;SysDelayUs(1);	   
	IIC2_SCL_H;SysDelayUs(1);	 
	while(IIC2_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>500)
		{
			IIC2_Stop();
			return 1;
		}
	}
	IIC2_SCL_L;   
	return 0;  
} 

void IIC2_Ack(void)
{
	IIC2_SCL_L;
	IIC2_SET_SDA_OUT();
	IIC2_SDA_L;
	SysDelayUs(20);
	IIC2_SCL_H;
	SysDelayUs(2);
	IIC2_SCL_L;
}
	    
void IIC2_NAck(void)
{
	IIC2_SCL_L;
	IIC2_SET_SDA_OUT();
	IIC2_SDA_H;
	SysDelayUs(5);
	IIC2_SCL_H;
	SysDelayUs(5);
	IIC2_SCL_L;
}			
		  
void IIC2_Send_Byte(uint8_t txd)
{                        
  uint8_t t;   
	IIC2_SET_SDA_OUT(); 	    
    IIC2_SCL_L;
    for(t=0;t<8;t++)
    {              
			  if((txd&0x80)>>7)
				{
					IIC2_SDA_H;
				}
				else
				{
				  IIC2_SDA_L;
				}
        txd<<=1; 
				SysDelayUs(5);
		    IIC2_SCL_H;
		    SysDelayUs(5);
		    IIC2_SCL_L;	
		    SysDelayUs(5);
    }	 
} 	    

uint8_t IIC2_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	IIC2_SET_SDA_IN();
  for(i=0;i<8;i++ )
  {
        IIC2_SCL_L; 
        SysDelayUs(5);
		    IIC2_SCL_H;
        receive<<=1;
    if(IIC2_READ_SDA)receive++;   
		    SysDelayUs(5);
  }					 
    if (!ack)
        IIC2_NAck();
    else
        IIC2_Ack(); 
    return receive;
}

/*温湿度驱动部分*/
Sensor soil_temp =
{
    "sth20_temp",
	  "0.1v",
	  SENSOR_TEMPERATURE,
	  DEGREE_CENTIGRADE, 
	  sht20_init,
	  sht20_configuration,
	  sht20_read_temp,
	  sht20_update_temp,
};

Sensor soil_humi =
{
    "sth20_humi",
	  "0.1v",
	  SENSOR_HUMIDIYT,
	  PERCENTAGE,
	  sht20_init,
	  sht20_configuration,
	  sht20_read_humi,
	  sht20_update_humi,
};

static uint8_t sht20_init_flag=0; ////避免温度和湿度的两次初始化
//static uint8_t htu21d_state=0; // 1: init error  |  0 :normal
static uint16_t temp=0; // 保存值 = 实际值*10 例如12.1° = 121
static uint16_t humi=0; 

#define   SHT20_ADR_W          0x80
#define   SHT20_ADR_R          0x81  
#define   SHT20_STATUS_MASK    0xfc
#define   SHT20_TEMP           0xE3 
#define   SHT20_HUMI           0xE5 
#define   SHT20_SOFT_RESET     0xFE 


uint8_t SHT20_SoftReset(void)
{
	uint8_t err=0;
	IIC2_Start();
  IIC2_Send_Byte(SHT20_ADR_W);
	err = IIC2_Wait_Ack();
	IIC2_Send_Byte(SHT20_SOFT_RESET);
	err = IIC2_Wait_Ack();
	IIC2_Stop();
	return err;
}

void sht20_init(void)
{
   IIC2_Init();
  // htu21d_state = 
	 SHT20_SoftReset(); 
	 sht20_init_flag=1;
}

void sht20_configuration(int time)
{
}

uint16_t  sht20_read_temp(void)
{
   return temp;
}

uint16_t sht20_read_humi(void)
{
  return humi;
}

SensorState  sht20_update_temp(void)
{
    uint8_t ack, tmp1, tmp2;
    uint16_t i=0;
	  float temp_f=0;
	  if(!sht20_init_flag)//初始化失败
		{
			//重新尝试初始化。尝试时间 为超时时间
		  temp = SENSOR_DEFAULT_TIMEOUT;
			return SENSOR_STATE_OK; 
		}
		IIC2_Start();
		IIC2_Send_Byte(SHT20_ADR_W);
		ack = IIC2_Wait_Ack();
		if(ack ==1)
		{
		   temp = SENSOR_DEFAULT_TIMEOUT;
			 return SENSOR_STATE_OK; 
		}
		IIC2_Send_Byte(SHT20_TEMP);
		ack = IIC2_Wait_Ack();
	  do {
        SysDelayMs(100);               
        IIC2_Start();				
        IIC2_Send_Byte(SHT20_ADR_R);	
			  i++;
			  ack = IIC2_Wait_Ack();
			  if(i==10)break;
    } while(ack!=0);
		tmp1 = IIC2_Read_Byte(1);
    tmp2 = IIC2_Read_Byte(1);
		IIC2_Read_Byte(0);
    IIC2_Stop();
		
		uint16_t ret = (tmp1 << 8) | (tmp2 & SHT20_STATUS_MASK);	
		if(ret ==0)
		{
		   temp = SENSOR_DEFAULT_DATA_UNUSUAL;
		   return SENSOR_STATE_OK; 
		}
    temp_f = ret*175.72/65536-46.85;		
		temp = (uint16_t)(temp_f*10.0);
		
		return SENSOR_STATE_OK; 
}


SensorState sht20_update_humi(void)
{
    uint8_t ack, tmp1, tmp2;
		uint16_t i=0;
    if(!sht20_init_flag)
		{
		
		  humi = SENSOR_DEFAULT_TIMEOUT;
			return SENSOR_STATE_OK; 
		}
		
    IIC2_Start();				
    IIC2_Send_Byte(SHT20_ADR_W);
		ack = IIC2_Wait_Ack();	
		if(ack ==1)
		{
		   humi = SENSOR_DEFAULT_TIMEOUT;
			 return SENSOR_STATE_OK; 
		}
		IIC2_Send_Byte(SHT20_HUMI);
		ack = IIC2_Wait_Ack();    
    do {
        SysDelayMs(100);               
        IIC2_Start();			
        IIC2_Send_Byte(SHT20_ADR_R);	
			  i++;
			  ack = IIC2_Wait_Ack();
			  if(i==10)break;
    } while(ack!=0);
  
    tmp1 = IIC2_Read_Byte(1);
    tmp2 = IIC2_Read_Byte(1);
    IIC2_Read_Byte(0);
    IIC2_Stop();
 
		uint16_t	ret = (tmp1 << 8) | (tmp2 & SHT20_STATUS_MASK);		
		if(ret ==0)
		{
		   humi = SENSOR_DEFAULT_DATA_UNUSUAL;
		   return SENSOR_STATE_OK; 
		}
	  humi =(uint16_t)(ret*125/65536-6);	
		
    return SENSOR_STATE_OK; 
}
