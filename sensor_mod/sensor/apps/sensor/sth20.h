#ifndef _SHT20_H_
#define _SHT20_H_

#include "sensor.h"


void sht20_init(void);
void sht20_configuration(int time);
SensorState  sht20_update_temp(void);
SensorState  sht20_update_humi(void);
uint16_t  sht20_read_temp(void);
uint16_t  sht20_read_humi(void);
#endif
