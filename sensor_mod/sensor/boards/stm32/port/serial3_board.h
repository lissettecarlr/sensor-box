#ifndef _SERIAL3_BOARD_
#define _SERIAL3_BOARD_

#include "stm32l1xx.h"

void Serial3BoardInit(void);
int GetSerial3BoardReciveDataSize(void);
void GetSerial3BoardReciveData(char *Data,int lenth);
void ClearSerial3BoardBuffer(void);
void Serial3BoardSendData(unsigned char *data,int lenth);
int isRcvData(void);

#endif
