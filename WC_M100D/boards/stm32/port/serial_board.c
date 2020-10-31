/*串口2 用于和传感器通信*/
#include "serial_board.h"
#include "fifo.h"
#include "uart.h"

#define UART_SENSOR_KEY 1

#define MAX_SER_RCV_DATA_FIFO_SIZE                 (200) 

static Fifo_t SerialBoardReciveFifo;
static char SerialBoardReciveDataBuffer[MAX_SER_RCV_DATA_FIFO_SIZE];

static void SerialBoardReceive( uint8_t notify, uint16_t value )
{

    if( notify == UART_NOTIFY_RX )
    {
        if( FifoPush( &SerialBoardReciveFifo, (char)value ) == 0 ) //存入数据
        {
					 //在休眠时段是接收不到数据的
           //push OK
					 //char a;
					 //FifoPop( &SerialBoardReciveFifo, &a );
					 //UartPutChar(1,a,false);
        }
				else
				{
				  //push error
				}
    }
		//else if()
		//  UART_NOTIFY_TX = 0,
    //  UART_NOTIFY_RX,
    //  UART_NOTIFY_CTS,
    //  UART_NOTIFY_PE
}

void SerialBoardSendData(unsigned char *data,int lenth)
{
   UartPutChars(UART_SENSOR_KEY,data,lenth);
}

void SerialBoardInit(void)
{
	  FifoInit( &SerialBoardReciveFifo, SerialBoardReciveDataBuffer, MAX_SER_RCV_DATA_FIFO_SIZE );
    UartInit( UART_SENSOR_KEY, RX_TX, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    UartSetInterrupt( UART_SENSOR_KEY, UART_IT_FLAG_RX, 0, 0, SerialBoardReceive);
}

//get buffer lenth
int GetSerialBoardReciveDataSize(void)
{ 
   return FifoDataLen(&SerialBoardReciveFifo);
}

//传入一个buffer，读取长度
void GetSerialBoardReciveData(char *Data,int lenth)
{
   for(int i=0;i<lenth;i++)
	   FifoPop( &SerialBoardReciveFifo, (Data+i) );
}

void ClearSerialBoardBuffer(void)
{
  FifoFlush(&SerialBoardReciveFifo);
}
