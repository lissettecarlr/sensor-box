/*串口2 用于和传感器通信*/
#include "serial3_board.h"
#include "fifo.h"
#include "uart.h"

#define UART_RADIO_ID 2


static Fifo_t Serial3BoardReciveFifo;
static char Serial3BoardReciveDataBuffer[200];
static uint8_t Rcvflag=0;

static void Serial3BoardReceive( uint8_t notify, uint16_t value )
{

	  if( notify == UART_NOTIFY_RX )
    {
        if( FifoPush( &Serial3BoardReciveFifo, (char)value ) == 0 ) //存入数据
        {
        }
    }
	
}

void Serial3BoardSendData(unsigned char *data,int lenth)
{
   UartPutChars(UART_RADIO_ID,data,lenth);
}

void Serial_recive_over()
{
    //SerialBoardSendData("123",strlen("123"));
	 Rcvflag=1;
}

int isRcvData(void)
{
	 uint8_t temp= GetSerial3BoardReciveDataSize();
	 if( temp ==0 )
	 {
	    Rcvflag=0;
	 }
   return Rcvflag;
}

void Serial3BoardInit(void)
{

	  FifoInit( &Serial3BoardReciveFifo, Serial3BoardReciveDataBuffer, 200 );
    UartInit( UART_RADIO_ID, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    UartSetInterrupt( UART_RADIO_ID, UART_IT_FLAG_RX, 0, 0, Serial3BoardReceive);
	  UartSetIdleCallbackInit(UART_RADIO_ID,Serial_recive_over); //空闲中断
	  Rcvflag =0;
}



//get buffer lenth
int GetSerial3BoardReciveDataSize(void)
{ 
   return FifoDataLen(&Serial3BoardReciveFifo);
}

//传入一个buffer，读取长度
void GetSerial3BoardReciveData(char *Data,int lenth)
{
   for(int i=0;i<lenth;i++)
	   FifoPop( &Serial3BoardReciveFifo, (Data+i) );
}

void ClearSerial3BoardBuffer(void)
{
	Rcvflag=0;
  FifoFlush(&Serial3BoardReciveFifo);
}
