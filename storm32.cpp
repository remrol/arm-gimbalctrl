#include "storm32.h"
extern "C"
{
	#include "uart.h"
};
#include "time.h"
#include <math.h>
#include <stdlib.h>

uint32_t        g_storm32LiveDataTimeStamp = 0;
Storm32LiveData g_storm32LiveData;


Storm32Status storm32_UpdateStatus()
{
	g_storm32LiveDataTimeStamp = millis();
	uint32_t timeout = g_storm32LiveDataTimeStamp + 100;
	uint8_t* stormData = (uint8_t*) & g_storm32LiveData;
	
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

			stormData[i] = (uint8_t) c;
			break;
		}
	}
	
	return ST32_UPDATE_OK;	
}


