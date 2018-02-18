#include "control.h"
#include "config.h"

extern "C"
{
#include "uart.h"	
#include "time.h"
};

char g_strbuf[32];

void sendConfig()
{
	sprintf_P(g_strbuf, PSTR("%ld,%d,%d,"), g_config.pulse_min, g_config.pulse_center, g_config.pulse_max);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_config.pulse_center_lo, g_config.pulse_center_hi, g_config.power);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d"), g_config.expo_percent, g_config.crc);
	uart_puts(g_strbuf);
	uart_puts_p( PSTR("\r\n"));
}

void control()
{
	// Timeout is 20 ms
//	uint32_t timeout = millis() + 20;

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
		case 'c': sendConfig(); break;
		}
	}
	while( 0 /*timeout > millis()*/ );	
}