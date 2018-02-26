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
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.pulse_min, g_config.pulse_max);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_config.pulse_center_lo, g_config.pulse_center_hi, g_config.power);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d"), g_config.expo_percent, g_config.crc);
	uart_puts(g_strbuf);
	uart_puts_p( PSTR("\r\n"));
}

void sendState()
{
	sprintf_P(g_strbuf, PSTR("%d,%ld,%d,"), g_state.motorPosition, g_state.lastPulseTime, g_state.pulseDuration);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_state.actualDirection, g_state.actualSpeed, g_state.speed);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%ld,%d"), g_state.timer1OverflowCount, g_state.ocr0);
	uart_puts(g_strbuf);
	uart_puts_p( PSTR("\r\n"));
}

void sendUptime()
{
	uint32_t msAbsolute = millis();
	uint32_t ms = msAbsolute;
	uint32_t sec  = ms / 1000;
	ms = ms - 1000 * sec;
	uint32_t minutes  = sec / 60;
	sec = sec - 60 * minutes;
	uint32_t hrs = minutes / 60;
	minutes = minutes - 60 * hrs;
	uint32_t days = hrs / 24;
	hrs = hrs - 24 * days;

	sprintf_P(g_strbuf, PSTR("%ld ms Uptime "), msAbsolute );
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d day %d hour %d min"), (uint16_t) days, (uint16_t) hrs, (uint16_t) minutes );
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR(" %d sec %d ms\r\n"), (uint16_t) sec, (uint16_t) ms );
	uart_puts(g_strbuf);
}

void sendInfo()
{
	uart_puts_p( PSTR("Gimbal ctrl build "));
	uart_puts_p( PSTR(__DATE__));
	uart_putc(' ');
	uart_puts_p( PSTR(__TIME__));
	uart_puts_p( PSTR("\r\n"));
}

void sendPower()
{
	sprintf_P(g_strbuf, PSTR("%d\r\n"), g_config.power);
	uart_puts(g_strbuf);
}

void receivePower()
{
	uint32_t timeout = millis() + 100;

	int16_t power = receiveInt16(timeout);
	if( power == INT16_MIN || power < 0 || power > 255 )
	{
		sprintf_P( g_strbuf, PSTR("ERR %d %d\r\n"), g_lastErr, power);
		uart_puts(g_strbuf);
		return;
	}

	g_config.power = power;
	sendPower();
}

void sendExpo()
{
	sprintf_P(g_strbuf, PSTR("%d\r\n"), g_config.expo_percent);
	uart_puts(g_strbuf);
}

void receiveExpo()
{
	uint32_t timeout = millis() + 100;

	int16_t expo = receiveInt16(timeout);
	if( expo == INT16_MIN || expo < 0 || expo > 100 )
	{
		sprintf_P( g_strbuf, PSTR("ERR %d %d\r\n"), g_lastErr, expo);
		uart_puts(g_strbuf);
		return;
	}

	g_config.expo_percent = expo;
	sendExpo();
}

void sendPulse()
{
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.pulse_min, g_config.pulse_max);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d"), g_config.pulse_center_lo, g_config.pulse_center_hi);
	uart_puts(g_strbuf);
	uart_puts_p( PSTR("\r\n"));
}

void receivePulse()
{
	// Timeout is 200 ms
	uint32_t timeout = millis() + 100;

	// Receive 
	int16_t min = receiveInt16(timeout);
	if( min == INT16_MIN || min < 100 || min > 2000 )
	{
		sprintf_P(g_strbuf, PSTR("MIN ERR %d\r\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}

	int16_t max = receiveInt16(timeout);
	if( max == INT16_MIN || max < 100 || max > 2000 || max <= min )
	{
		sprintf_P(g_strbuf, PSTR("MAX ERR %d\r\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}

	int16_t clo = receiveInt16(timeout);
	if( clo == INT16_MIN || clo < 100 || clo > 2000 ||  clo <= min || clo >= max )
	{
		sprintf_P(g_strbuf, PSTR("CLO ERR %d\r\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}

	int16_t chi = receiveInt16(timeout);
	if( chi == INT16_MIN || chi < 100 || chi > 2000 ||  chi <= min || chi >= max || chi < clo)
	{
		sprintf_P(g_strbuf, PSTR("CLO ERR %d\r\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}


	// Write to config
	g_config.pulse_min = min;
	g_config.pulse_max = max;
	g_config.pulse_center_lo = clo;
	g_config.pulse_center_hi = chi;
	
	// Send back received values
	sendPulse();

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
		case 'c': 
			sendConfig(); 
			break;
			
		case 's': 
			sendState(); 
			break;
			
		case 'W': 
			configEepromSave(); 
			uart_puts_p( PSTR("1\r\n")); 
			break;
			
		case 'u':
			sendUptime(); 
			break;
			
		case 'i':
			sendInfo(); 
			break;
			
		case 'P':
			receivePower();
			break;
			
		case 'p':
			sendPower();
			break;
			
		case 'e':
			sendExpo();
			break;
			
		case 'E':
			receiveExpo();
			break;
			
		case 'l':
			sendPulse();
			break;
			
		case 'L':
			receivePulse();
			break;
			
		}
	}
	while( timeout > millis() );	
}