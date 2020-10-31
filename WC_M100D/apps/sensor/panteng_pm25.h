
#ifndef __PANTENG_PM2_5_H__
#define __PANTENG_PM2_5_H__

#include "sensor.h"

void        PANTENG_pm2_5_init(void);
void        PANTENG_pm2_5_configuration(int time);
uint16_t    PANTENG_pm2_5_read(void);
SensorState PANTENG_pm2_5_update(void);



#endif
