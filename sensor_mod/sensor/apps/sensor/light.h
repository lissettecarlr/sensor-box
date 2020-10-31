#ifndef _SENSOR_LIGHT_H
#define _SENSOR_LIGHT_H


#include "sensor.h"


void        light_r_init(void);
void        light_r_configuration(int time);
uint16_t    light_r_read(void);
SensorState light_r_update(void);


#endif
