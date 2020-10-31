#ifndef _SENSOR_BATTERY_H
#define _SENSOR_BATTERY_H


#include "sensor.h"


void        battery_r_init(void);
void        battery_r_configuration(int time);
uint16_t    battery_r_read(void);
SensorState battery_r_update(void);


#endif
