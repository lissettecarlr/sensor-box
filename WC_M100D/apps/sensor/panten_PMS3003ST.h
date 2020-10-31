#ifndef _PANTEN_PMS3003ST_
#define _PANTEN_PMS3003ST_

#include "sensor.h"



void panten_PMS3003ST_init(void);
void panten_PMS3003ST_configuration(int time);
uint16_t panten_PMS3003ST_read(void);
SensorState panten_PMS3003ST_update(void);


#endif
