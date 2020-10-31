#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "type.h"

//非特异传感器超时时间
#define SENSOR_TIMEOUT_DEFAULT        5 

//错误代码
enum{
  SENSOR_DEFAULT_TIMEOUT       = 0xff92,        //超时       -110
  SENSOR_DEFAULT_CRC_ERROR     = 0xff91,       //CRC错误     -111
  SENSOR_DEFAULT_DATA_UNUSUAL  = 0xff90,        //数据异常   -112
};

//单位
enum{
    UG_M3              = 1,
	  MG_M3              = 2,
	  VOLTAGE            = 3,
		DEGREE_CENTIGRADE  = 4,
		PERCENTAGE         = 5,
		LUX								 = 6,
};


typedef enum
{
	SENSOR_STATE_READY =1,	           //准备阶段
	SENSOR_STATE_BUSY,		           //数据更新阶段
	SENSOR_STATE_OK,			       //更新完成阶段
	SENSOR_STATE_OTHER,                //预留状态
}SensorState;

//传感器类型分类
enum
{
	SENSOR_PM2_5        =1,	        
	SENSOR_TEMPERATURE  =2,		    
	SENSOR_HUMIDIYT     =3,	
	SENSOR_LIGHT        =4,
	SENSOR_BATTERY        =5,
};


typedef struct
{
	const char     	name[20]; 
	const char     	version[20];
	const uint8_t  	type;        //传感器类型
	const uint16_t 	unit_type;   //数据单位
	
    /*!
     * \brief sensor init.
     */
    void       ( *init )( void );
    /*!
	 * \param [IN] The longest time-consuming
     * \brief  configuration ,default SENSOR_TIMEOUT_DEFAULT
     */
    void       ( *configuration )( int time);
	 /*!
     * \brief read the stored in u16 of data,not update.
     */
    uint16_t   ( *read )( void );
     /*!
     * \brief update data
     */
    SensorState   ( *update )( void );
	
     
}Sensor;

#endif
