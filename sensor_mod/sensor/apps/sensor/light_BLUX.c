#include "light_BLUX.h"
#include "i2c.h"
#include "systick.h"
#include "board.h"
Sensor light =
{
    "B_LUX_SENSOR",
	  "0.1v",
	  SENSOR_LIGHT,
	  LUX, 
	  B_Lux_init,
	  B_Lux_configuration,
	  B_Lux_read,
	  B_Lux_update,
};

static uint8_t B_Lux_init_flag=0; 
static uint16_t B_Lux=0; 
static uint8_t B_error_flag=0;


void B_Lux_init(void)
{
   IIC_Init();
//	 B_Lux_SoftReset(); 
	 B_Lux_init_flag=1;
}

void B_Lux_configuration(int time)
{
}

uint16_t  B_Lux_read(void)
{
   return B_Lux;
}

void bh_data_send(uint8_t command)  
{  
	  int i=0,ack;
	  B_error_flag =0;
    do{  
			SysDelayMs(100); 
      IIC_Start();                     
      IIC_Send_Byte(BHAddWrite);    
      i++;
      ack = IIC_Wait_Ack();		
      if(i>=10)
			{
				B_error_flag =1;
				break;
			}				
    }while(ack!=0);    
		
    IIC_Send_Byte(command);          
    IIC_Wait_Ack();                  
    IIC_Stop();                      
}


uint16_t bh_data_read(void)  
{  
    uint16_t buf;  
    IIC_Start();                       
    IIC_Send_Byte(BHAddRead);         
    IIC_Wait_Ack();                   
    buf=IIC_Read_Byte(1);              
    buf=buf<<8;                       
    buf+=0x00ff&IIC_Read_Byte(0);     
    IIC_Stop();                       
    return buf;   
}  

SensorState  B_Lux_update(void)
{
 
    // float datalx = 0; 
	  if(!B_Lux_init_flag)
		{

		  B_Lux = SENSOR_DEFAULT_TIMEOUT;
			return SENSOR_STATE_OK; 
		}
		
		bh_data_send(BHPowOn);    
    bh_data_send(BHReset);  
    bh_data_send(BHModeH2); 
		DelayMs(180);           
 
    B_Lux =bh_data_read()/1.2; 
		if(B_error_flag ==1)
			B_Lux = -110;
		DelayMs(500); 
		

	return SENSOR_STATE_OK;
}

