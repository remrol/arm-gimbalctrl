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
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_config.pulse_dband_lo, g_config.pulse_dband_hi, g_config.pwm_scale_factor);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_config.speed_smooth_factor, g_config.power, g_config.expo_percent);
	uart_puts(g_strbuf);	
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.process_pulse_interval_ms, g_config.process_speedsmooth_interval_ms);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\r\n"), g_config.mot_stop_nopulse_timeout_ms, g_config.mot_disable_stopped_timeout_ms, g_config.crc);
	uart_puts(g_strbuf);
}

void sendState()
{
	sprintf_P(g_strbuf, PSTR("%ld,%d,%ld,"), millis(), g_state.vMotorPosition, g_state.vPulseTimeStamp );
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_state.pulseDuration, g_state.motorDirection, g_state.motorSpeed );
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d\r\n"), g_state.speed, g_state.ocr0);
	uart_puts(g_strbuf);
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

void sendPulseRanges()
{
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.pulse_min, g_config.pulse_max);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d\r\n"), g_config.pulse_dband_lo, g_config.pulse_dband_hi);
	uart_puts(g_strbuf);
}

void receivePulseRanges()
{
	// Timeout is 200 ms
	uint32_t timeout = millis() + 100;

	// Receive 
	int16_t min = receiveInt16(timeout);
	if( min == INT16_MIN || min < 100 || min > 5000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\r\n"), g_lastErr, min);
		uart_puts( g_strbuf );
		return;
	}

	int16_t max = receiveInt16(timeout);
	if( max == INT16_MIN || max < 100 || max > 5000 || max <= min )
	{
		sprintf_P(g_strbuf, PSTR("ERR1 %d %d\r\n"), g_lastErr, max);
		uart_puts( g_strbuf );
		return;
	}

	int16_t clo = receiveInt16(timeout);
	if( clo == INT16_MIN || clo < 100 || clo > 5000 ||  clo <= min || clo >= max )
	{
		sprintf_P(g_strbuf, PSTR("ERR2 %d %d\r\n"), g_lastErr, clo);
		uart_puts( g_strbuf );
		return;
	}

	int16_t chi = receiveInt16(timeout);
	if( chi == INT16_MIN || chi < 100 || chi > 5000 ||  chi <= min || chi >= max || chi < clo)
	{
		sprintf_P(g_strbuf, PSTR("ERR3 %d %d\r\n"), g_lastErr, chi);
		uart_puts( g_strbuf );
		return;
	}


	// Write to config
	g_config.pulse_min = min;
	g_config.pulse_max = max;
	g_config.pulse_dband_lo = clo;
	g_config.pulse_dband_hi = chi;
	
	// Send back received values
	sendPulseRanges();
}

void sendMotorParams()
{
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\r\n"), g_config.power, g_config.pwm_scale_factor, g_config.expo_percent);
	uart_puts(g_strbuf);
}

void receiveMotorParams()
{
	// Timeout is 200 ms
	uint32_t timeout = millis() + 100;

	int16_t power = receiveInt16(timeout);
	if( power == INT16_MIN || power < 0 || power > 255 )
	{
		sprintf_P( g_strbuf, PSTR("ERR0 %d %d\r\n"), g_lastErr, power);
		uart_puts(g_strbuf);
		return;
	}

	int16_t scf = receiveInt16(timeout);
	if( scf == INT16_MIN || scf < 0 || scf > 10000 )
	{
		sprintf_P( g_strbuf, PSTR("ERR1 %d %d\r\n"), g_lastErr, scf);
		uart_puts(g_strbuf);
		return;
	}

	int16_t expo = receiveInt16(timeout);
	if( expo == INT16_MIN || expo < 0 || expo > 100 )
	{
		sprintf_P( g_strbuf, PSTR("ERR2 %d %d\r\n"), g_lastErr, expo);
		uart_puts(g_strbuf);
		return;
	}

	g_config.power = power;
	g_config.pwm_scale_factor = scf;
	g_config.expo_percent = expo;

	sendMotorParams();
}

void sendProcessing()
{
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\r\n"), g_config.process_pulse_interval_ms, g_config.process_speedsmooth_interval_ms, g_config.speed_smooth_factor);
	uart_puts(g_strbuf);
}

void receiveProcessing()
{
	// Timeout is 200 ms
	uint32_t timeout = millis() + 100;

	// Receive
	int16_t pulseInterval = receiveInt16(timeout);
	if( pulseInterval == INT16_MIN || pulseInterval < 0 || pulseInterval > 10000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\r\n"), g_lastErr, pulseInterval);
		uart_puts( g_strbuf );
		return;
	}

	int16_t speedSmoothInterval = receiveInt16(timeout);
	if( speedSmoothInterval == INT16_MIN || speedSmoothInterval < 0 || speedSmoothInterval > 10000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR1 %d\r\n"), g_lastErr, speedSmoothInterval);
		uart_puts( g_strbuf );
		return;
	}

	int16_t speedSmoothFactor = receiveInt16(timeout);
	if( speedSmoothFactor == INT16_MIN || speedSmoothFactor < 0 || speedSmoothFactor > 127 )
	{
		sprintf_P(g_strbuf, PSTR("ERR2 %d %d\r\n"), g_lastErr, speedSmoothFactor);
		uart_puts( g_strbuf );
		return;
	}

	// Write to config
	g_config.process_pulse_interval_ms = pulseInterval;
	g_config.process_speedsmooth_interval_ms = speedSmoothInterval;
	g_config.speed_smooth_factor = speedSmoothFactor;
	
	// Send back received values
	sendProcessing();
}

void sendTimeouts()
{
	sprintf_P(g_strbuf, PSTR("%d,%d\r\n"), g_config.mot_stop_nopulse_timeout_ms, g_config.mot_disable_stopped_timeout_ms);
	uart_puts(g_strbuf);
}

void receiveTimeouts()
{
	// Timeout is 200 ms
	uint32_t timeout = millis() + 100;

	// Receive
	int16_t mot_stop_nopulse_timeout_ms = receiveInt16(timeout);
	if( mot_stop_nopulse_timeout_ms == INT16_MIN || mot_stop_nopulse_timeout_ms < 0 || mot_stop_nopulse_timeout_ms > 30000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\r\n"), g_lastErr, mot_stop_nopulse_timeout_ms);
		uart_puts( g_strbuf );
		return;
	}

	int16_t mot_disable_stopped_timeout_ms = receiveInt16(timeout);
	if( mot_disable_stopped_timeout_ms == INT16_MIN || mot_disable_stopped_timeout_ms < 0 || mot_disable_stopped_timeout_ms > 30000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR1 %d\r\n"), g_lastErr, mot_disable_stopped_timeout_ms);
		uart_puts( g_strbuf );
		return;
	}

	g_config.mot_stop_nopulse_timeout_ms = mot_stop_nopulse_timeout_ms;
	g_config.mot_disable_stopped_timeout_ms = mot_disable_stopped_timeout_ms;

	sendTimeouts();
}

void sendDiagnostics()
{
	sprintf_P(g_strbuf, PSTR("%d,%d\r\n"), g_state.diag0, g_state.diag1);
	uart_puts(g_strbuf);
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
			sendConfig(); 	break;
			
		case 's': 
			sendState(); 	break;
			
		case 'W': 
			configEepromSave(); uart_puts_p( PSTR("1\r\n")); break;
		case 'd':
			configLoadDefaults(); uart_puts_p( PSTR("1\r\n")); break;
			
		case 'u':
			sendUptime(); 	break;
			
		case 'i':
			sendInfo(); 	break;
				
		case 'l':
			sendPulseRanges(); 	break;
		case 'L':
			receivePulseRanges(); break;

		case 'a':
			sendProcessing(); break;
		case 'A':
			receiveProcessing(); break;

		case 'b':
			sendDiagnostics(); break;

		case 'm':
			sendMotorParams(); break;
		case 'M':
			receiveMotorParams(); break;

		case 't':
			sendTimeouts(); break;
		case 'T':
			receiveTimeouts(); break;	
		}
	}
	while( timeout > millis() );	
}