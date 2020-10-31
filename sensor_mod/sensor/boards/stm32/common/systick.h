#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "stm32l1xx.h"

void sysclk_init(void);
double GetNowTime(void);
int ClockTool(double *record,double timeout);
void SysDelayUs(uint16_t nus);
void SysDelayMs(uint16_t nms);

#endif
