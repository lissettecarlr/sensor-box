#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "type.h"

typedef enum
{
  CONNECTION_OFF  =1,
  CONNECTION_ON   =2,
}ConnectionStatus;

typedef enum
{
  MODULE_SEND_OK     =1,
  MODULE_SEND_FAIL   =2,
}SendAckInfo;

enum
{
	COMMUNICATION_LORA        =1,	        
	COMMUNICATION_FSK         =2,		    
	COMMUNICATION_WIFI        =3,	
};

typedef struct
{
	const char     name[20]; 
	const char     version[20];
	const uint8_t  type;      

  void      ( *init )( void );
	ConnectionStatus   ( *join )( void );
	void   ( *close)( void );
	ConnectionStatus   ( *GetConnectionSatus )( void );
	SendAckInfo  (* Send )(uint8_t* data , int lenth);

}communication;

#endif
