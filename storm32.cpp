#include "storm32.h"
extern "C"
{
	#include "uart.h"
};
#include "time.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

uint32_t        g_storm32LiveDataTimeStamp = 0;
Storm32LiveData g_storm32LiveData;


#define X25_INIT_CRC 0xffff
#define X25_VALIDATE_CRC 0xf0b8
 
static inline void crc_accumulate(uint8_t data, uint16_t *crcAccum)
{
	/*Accumulate one byte of data into the CRC*/
    uint8_t tmp;
 
    tmp=data ^ (uint8_t)(*crcAccum &0xff);
    tmp^= (tmp<<4);
    *crcAccum = (*crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);
}
 

static inline void crc_init(uint16_t* crcAccum)
{
    *crcAccum = X25_INIT_CRC;
}


void storm32_Init()
{
	g_storm32LiveDataTimeStamp = 0;
	memset(&g_storm32LiveData, 0, sizeof(Storm32LiveData));
}

Storm32Status storm32_UpdateStatus()
{
	uint32_t timeStamp = millis();
	uint32_t timeout = timeStamp + 100;
	uint8_t stormData[sizeof(Storm32LiveData)];
	uint16_t crc;
	
	crc_init(&crc);
	
	uart1_putc('d');
	
	for( uint8_t i = 0; i < sizeof(Storm32LiveData); ++i )
	{
		uint16_t c;
		
		while( true)
		{
			c = uart1_getc();
			
			// No data, check for timeout
			if( c & UART_NO_DATA)
			{
				// Detect timeout
				if( millis() > timeout )
					return ST32_UPDATE_TIMEOUT;
				
				continue;
			}

			// Error, return with error
			if( c & 0xff00 )
				return ST32_UPDATE_UARTERROR;

			crc_accumulate((uint8_t) c, &crc);
			stormData[i] = (uint8_t) c;
			break;
		}
	}
	
	Storm32LiveData* st32Data = (Storm32LiveData*) stormData;
	
//	if(st32Data->crc != crc)
//		return ST32_UPDATE_CRCERROR;
	
	if( st32Data->endChar != 'o' )
		return ST32_UPDATE_ENDCHARERROR;
	
	g_storm32LiveDataTimeStamp = timeStamp;
	memcpy(&g_storm32LiveData, st32Data, sizeof(Storm32LiveData));
	return ST32_UPDATE_OK;	
}


