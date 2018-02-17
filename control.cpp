#include "control.h"
#include "config.h"
extern "C"
{
#include "uart.h"	
};



void control()
{
	// Timeout is 100 ms
//	uint32_t timeout = millis() + 100;

	do
	{
		uint16_t c = uart_getc();
		
		// No data -> return immediately
		if( c & UART_NO_DATA)
			return;
		// Error, continue read loop
		if( c & 0xff00 )
			continue;
		
		// First byte denotes a command
		switch(c)
		{
		}
	}
	while( 1 /*timeout > millis()*/ );	
}