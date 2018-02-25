#define __STDC_LIMIT_MACROS
#include "control.h"
#include "config.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

extern "C"
{
#include "uart.h"	
#include "time.h"
};

char g_strbuf[32];

uint16_t g_lastErr = 0;

// Receives float or NAN in case of timeout or an error
float receiveFloat( uint32_t _timeout)
{
	int32_t num = 0;	// Numerator
	int32_t den = 1;	// Denominator
	int8_t  sign = 1;	// Sign
	uint8_t digits = 0;	// Digits read so far
	int8_t  hasDot = 0;	// Indicates "."

	while(1)
	{
		uint16_t c = uart_getc();
		
		// No data, check for timeout
		if( c & UART_NO_DATA)
		{
			// Detect timeout
			if( millis() > _timeout )
			{
				g_lastErr = 1;	// Error is timeout
				return NAN;
			}
			
			continue;
		}

		// Error, return with error
		if( c & 0xff00 )
		{
			g_lastErr = ( c & 0xff00 );	// Error is UART error flag
			return NAN;
		}

		// If zero digits then recognize sign or whitespace
		if( digits == 0 )
		{
			if( c == '+' )
			{
				sign = 1;
				continue;
			}
			else if( c == '-' )
			{
				sign = -1;
				continue;
			}
			else if( c == ' ' || c == '\t')
			{
				continue;
			}
		}

		if( c == '.')
		{
			if( hasDot )
			{
				g_lastErr = 2;	// Error is multiple dot
				return NAN;	// Cannot have multiple dots.
			}

			hasDot = 1;	// Signal dot.
		}
		else if( c >= '0' && c <= '9' ) // Detect and consume digit
		{
			num = 10 * num + ( c - '0' );
			digits += 1;

			// Multiply denominator
			if( hasDot )
			den = 10 * den;
		}
		else
		{
			break;
		}
	}

	// If no digits then return error.
	if( digits == 0 )
	{
		g_lastErr = 3;	// Error is no digits
		return NAN;
	}

	// Return value with sign applied
	return ( float )( num * sign ) / ( float ) den;
}


// Receives int16 or INT16_MIN in case of timeout or an error
int16_t receiveInt16( uint32_t _timeout)
{
	int16_t value = 0;		// Accumulated value
	int8_t sign = 1;		// Sign
	uint8_t digits = 0;		// Digits read so far

	while(1)
	{
		uint16_t c = uart_getc();
		
		// No data, check for timeout
		if( c & UART_NO_DATA)
		{
			// Detect timeout
			if( millis() > _timeout )
			{
				g_lastErr = 1;	// Error is timeout
				return INT16_MIN;
			}
			
			continue;
		}

		// Error, return with error
		if( c & 0xff00 )
		{
			g_lastErr = ( c & 0xff00 );	// Error is UART error flag
			return INT16_MIN;
		}

		// If zero digits then recognize sign or whitespace
		if( digits == 0 )
		{
			if( c == '+' )
			{
				sign = 1;
				continue;
			}
			else if( c == '-' )
			{
				sign = -1;
				continue;
			}
			else if( c == ' ' || c == '\t')
			{
				continue;
			}
		}

		// Detect and consume digit
		if( c >= '0' && c <= '9' )
		{
			value = 10 * value + ( c - '0' );
			digits += 1;
		}
		else
		{
			break;
		}
	}

	// If no digits then return error.
	if( digits == 0 )
	{
		g_lastErr = 3;	// Error is no digits
		return INT16_MIN;
	}

	// Return value with sign applied
	return value * sign;
}

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
	uint32_t timeout = millis() + 20;

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
	while( timeout > millis() );	
}