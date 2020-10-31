#ifndef _HTU21D_H
#define _HTU21D_H

#include "sensor.h"

void         htu21d_init(void);
void         htu21d_configuration(int time);
uint16_t     htu21d_read_temp(void);
uint16_t     htu21d_read_humi(void);
SensorState  htu21d_update_temp(void);
SensorState  htu21d_update_humi(void);

#endif
