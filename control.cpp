#define __STDC_LIMIT_MACROS
#include "control.h"
#include "config.h"
#include "storm32.h"
#include "debug.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

extern "C"
{
#include "uart.h"	
};

#include "time.h"

char g_strbuf[32];

uint16_t g_lastErr = 0;


void sendFloat( float f )
{
	dtostrf(f, 8, 6, g_strbuf);
	uart_puts(g_strbuf);
}

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
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,%d,"), g_config.speed_normal_smooth_factor, g_config.yaw_speed_smooth_factor, g_config.power, g_config.expo_percent);
	uart_puts(g_strbuf);	
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.process_pulse_interval_ms, g_config.process_speedsmooth_interval_ms);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,"), g_config.mot_stop_nopulse_timeout_ms, g_config.mot_disable_stopped_timeout_ms, g_config.storm32_update_inteval_ms);
	uart_puts(g_strbuf);
	sendFloat(g_config.yawPID_p);
	uart_puts_p( PSTR(","));
	sendFloat(g_config.yawPID_i);
	uart_puts_p( PSTR(","));
	sendFloat(g_config.yawPID_d);
	uart_puts_p( PSTR(","));
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.yawMaxSpeed, g_config.sensors_update_interval_ms);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d\n"), g_config.crc);
	uart_puts(g_strbuf);

}

void sendState()
{
	sprintf_P(g_strbuf, PSTR("%ld,%d,%ld,%ld,"), millis(), g_state.vMotorPosition, g_state.vPulse1TimeStamp, g_state.vPulse3TimeStamp  );
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d,%d,"), g_state.pulse1Duration, g_state.pulse3Duration, g_state.motorDirection, g_state.motorSpeed );
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d\n"), g_state.yawCtrlSpeed, g_state.ocr0);
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
	sprintf_P(g_strbuf, PSTR(" %d sec %d ms\n"), (uint16_t) sec, (uint16_t) ms );
	uart_puts(g_strbuf);
}

void sendInfo()
{
	uart_puts_p( PSTR("Gimbal ctrl build "));
	uart_puts_p( PSTR(__DATE__));
	uart_putc(' ');
	uart_puts_p( PSTR(__TIME__));
	uart_puts_p( PSTR("\n"));
}

void sendPulseRanges()
{
	sprintf_P(g_strbuf, PSTR("%d,%d,"), g_config.pulse_min, g_config.pulse_max);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d\n"), g_config.pulse_dband_lo, g_config.pulse_dband_hi);
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
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\n"), g_lastErr, min);
		uart_puts( g_strbuf );
		return;
	}

	int16_t max = receiveInt16(timeout);
	if( max == INT16_MIN || max < 100 || max > 5000 || max <= min )
	{
		sprintf_P(g_strbuf, PSTR("ERR1 %d %d\n"), g_lastErr, max);
		uart_puts( g_strbuf );
		return;
	}

	int16_t clo = receiveInt16(timeout);
	if( clo == INT16_MIN || clo < 100 || clo > 5000 ||  clo <= min || clo >= max )
	{
		sprintf_P(g_strbuf, PSTR("ERR2 %d %d\n"), g_lastErr, clo);
		uart_puts( g_strbuf );
		return;
	}

	int16_t chi = receiveInt16(timeout);
	if( chi == INT16_MIN || chi < 100 || chi > 5000 ||  chi <= min || chi >= max || chi < clo)
	{
		sprintf_P(g_strbuf, PSTR("ERR3 %d %d\n"), g_lastErr, chi);
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
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\n"), g_config.power, g_config.pwm_scale_factor, g_config.expo_percent);
	uart_puts(g_strbuf);
}

void receiveMotorParams()
{
	// Timeout is 200 ms
	uint32_t timeout = millis() + 100;

	int16_t power = receiveInt16(timeout);
	if( power == INT16_MIN || power < 0 || power > 255 )
	{
		sprintf_P( g_strbuf, PSTR("ERR0 %d %d\n"), g_lastErr, power);
		uart_puts(g_strbuf);
		return;
	}

	int16_t scf = receiveInt16(timeout);
	if( scf == INT16_MIN || scf < 0 || scf > 10000 )
	{
		sprintf_P( g_strbuf, PSTR("ERR1 %d %d\n"), g_lastErr, scf);
		uart_puts(g_strbuf);
		return;
	}

	int16_t expo = receiveInt16(timeout);
	if( expo == INT16_MIN || expo < 0 || expo > 100 )
	{
		sprintf_P( g_strbuf, PSTR("ERR2 %d %d\n"), g_lastErr, expo);
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
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\n"), g_config.process_pulse_interval_ms, g_config.process_speedsmooth_interval_ms, g_config.speed_normal_smooth_factor);
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
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\n"), g_lastErr, pulseInterval);
		uart_puts( g_strbuf );
		return;
	}

	int16_t speedSmoothInterval = receiveInt16(timeout);
	if( speedSmoothInterval == INT16_MIN || speedSmoothInterval < 0 || speedSmoothInterval > 10000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR1 %d\n"), g_lastErr, speedSmoothInterval);
		uart_puts( g_strbuf );
		return;
	}

	int16_t speedSmoothFactor = receiveInt16(timeout);
	if( speedSmoothFactor == INT16_MIN || speedSmoothFactor < 0 || speedSmoothFactor > 127 )
	{
		sprintf_P(g_strbuf, PSTR("ERR2 %d %d\n"), g_lastErr, speedSmoothFactor);
		uart_puts( g_strbuf );
		return;
	}

	// Write to config
	g_config.process_pulse_interval_ms = pulseInterval;
	g_config.process_speedsmooth_interval_ms = speedSmoothInterval;
	g_config.speed_normal_smooth_factor = speedSmoothFactor;
	
	// Send back received values
	sendProcessing();
}

void sendTimeouts()
{
	sprintf_P(g_strbuf, PSTR("%d,%d\n"), g_config.mot_stop_nopulse_timeout_ms, g_config.mot_disable_stopped_timeout_ms);
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
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\n"), g_lastErr, mot_stop_nopulse_timeout_ms);
		uart_puts( g_strbuf );
		return;
	}

	int16_t mot_disable_stopped_timeout_ms = receiveInt16(timeout);
	if( mot_disable_stopped_timeout_ms == INT16_MIN || mot_disable_stopped_timeout_ms < 0 || mot_disable_stopped_timeout_ms > 30000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR1 %d\n"), g_lastErr, mot_disable_stopped_timeout_ms);
		uart_puts( g_strbuf );
		return;
	}

	g_config.mot_stop_nopulse_timeout_ms = mot_stop_nopulse_timeout_ms;
	g_config.mot_disable_stopped_timeout_ms = mot_disable_stopped_timeout_ms;

	sendTimeouts();
}


void sendSensors()
{
	sprintf_P(g_strbuf, PSTR("%ld,%d,%ld,"), g_state.baroTimeStamp, g_state.baroTempX10, g_state.baroPressure);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%ld,%d,%d,%d,"), g_state.magnTimeStamp, g_state.magnX, g_state.magnY, g_state.magnZ);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%ld,%d,%d,%d,"), g_state.mpuEventTimeStamp, g_state.mpuAccelX, g_state.mpuAccelY, g_state.mpuAccelZ);
	uart_puts(g_strbuf);
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\n"), g_state.mpuGyroX, g_state.mpuGyroY, g_state.mpuGyroZ);
	uart_puts(g_strbuf);
}

// InputSrcPitch
void updateStorm32LiveData()
{
	Storm32Status status = storm32UpdateStatus();
	
	if( status != ST32_UPDATE_OK )
	{
		sprintf_P(g_strbuf, PSTR("ERR %d\n"), (int) status);
		uart_puts(g_strbuf);
	}
	else
	{
		sprintf_P(g_strbuf, PSTR("%ld\n"), g_storm32LiveDataTimeStamp);
		uart_puts(g_strbuf);	
	}
}


void readStorm32LiveData()
{
	// Timeout is 100 ms
	uint32_t timeout = millis() + 100;

	// Receive
	int16_t dataOffset = receiveInt16(timeout);
	if( dataOffset == INT16_MIN || dataOffset < 0 || dataOffset > STORM32_FIELDSCOUNT - 2 )
	{
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\n"), g_lastErr, dataOffset);
		uart_puts( g_strbuf );
		return;
	}
	
	int16_t* data = ( (int16_t*) ( &g_storm32LiveData) ) + dataOffset;
	sprintf_P(g_strbuf, PSTR("%d,%d,%d\n"), data[0], data[1],  data[2] );
	uart_puts(g_strbuf);	
}

void sendDebug()
{
	// Timeout is 100 ms
	uint32_t timeout = millis() + 100;

	// Receive
	int16_t dataOffset = receiveInt16(timeout);
	if( dataOffset == INT16_MIN || dataOffset < 0 || dataOffset > (int16_t) ( sizeof(g_debug) / 2 ) )
	{
		sprintf_P(g_strbuf, PSTR("ERR0 %d %d\n"), g_lastErr, dataOffset);
		uart_puts( g_strbuf );
		return;
	}

	int16_t* data = (int16_t*) &g_debug ;
	sprintf_P(g_strbuf, PSTR("%d\n"), data[dataOffset] );
	uart_puts(g_strbuf);
}

void sendYaw()
{
	sendFloat(g_config.yawPID_p);
	uart_puts_p( PSTR(","));
	sendFloat(g_config.yawPID_i);
	uart_puts_p( PSTR(","));
	sendFloat(g_config.yawPID_d);
	sprintf_P(g_strbuf, PSTR(",%d,%d,%d\n"), (int) g_config.yaw_speed_smooth_factor, (int) g_config.storm32_update_inteval_ms, (int) g_config.yawMaxSpeed );
	uart_puts(g_strbuf);
}

void receiveYaw()
{
	// Timeout is 100 ms
	uint32_t timeout = millis() + 100;
	
	float p = receiveFloat(timeout);
	if( isnan(p) || p <= 0)
	{
		sprintf_P(g_strbuf, PSTR("P ERR %d\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}		
	
	float i = receiveFloat(timeout);
	if( isnan(i) || i <= 0)
	{
		sprintf_P(g_strbuf, PSTR("I ERR %d\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}	
	
	float d = receiveFloat(timeout);
	if( isnan(d) || d <= 0)
	{
		sprintf_P(g_strbuf, PSTR("D ERR %d\n"), g_lastErr);
		uart_puts( g_strbuf );
		return;
	}
	
	int16_t yaw_speed_smooth_factor = receiveInt16(timeout);
	if( yaw_speed_smooth_factor == INT16_MIN || yaw_speed_smooth_factor <= 0 || yaw_speed_smooth_factor > 127 )
	{
		sprintf_P(g_strbuf, PSTR("ERR2 %d %d\n"), g_lastErr, yaw_speed_smooth_factor);
		uart_puts( g_strbuf );
		return;
	}	
	
	int16_t storm32_update_inteval_ms = receiveInt16(timeout);
	if( storm32_update_inteval_ms == INT16_MIN || storm32_update_inteval_ms <= 0 || storm32_update_inteval_ms > 1000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR3 %d %d\n"), g_lastErr, storm32_update_inteval_ms);
		uart_puts( g_strbuf );
		return;
	}	
	
	int16_t yawMaxSpeed = receiveInt16(timeout);
	if( yawMaxSpeed == INT16_MIN || yawMaxSpeed <= 0 || yawMaxSpeed > 1000 )
	{
		sprintf_P(g_strbuf, PSTR("ERR4 %d %d\n"), g_lastErr, yawMaxSpeed);
		uart_puts( g_strbuf );
		return;
	}
	
	g_config.yawPID_p = p;
	g_config.yawPID_i = i;
	g_config.yawPID_d = d;
	g_config.yaw_speed_smooth_factor = yaw_speed_smooth_factor;
	g_config.storm32_update_inteval_ms = storm32_update_inteval_ms;
	g_config.yawMaxSpeed = yawMaxSpeed;
	
	sendYaw();
	
	if( g_state.yawStabilizeMode )
	{
		PID_SetTunings( g_config.yawPID_p, g_config.yawPID_i, g_config.yawPID_d, &g_state.yawPID );		
	}
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
			configEepromSave(); uart_puts_p( PSTR("1\n")); break;
		case 'd':
			configLoadDefaults(); uart_puts_p( PSTR("1\n")); break;
			
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

		case 'm':
			sendMotorParams(); break;
		case 'M':
			receiveMotorParams(); break;

		case 't':
			sendTimeouts(); break;
		case 'T':
			receiveTimeouts(); break;	
			
		case 'e':
			sendSensors(); break;
			
		case 'f':
			updateStorm32LiveData(); break;
			
		case 'g':
			readStorm32LiveData(); break;
			
		case 'h':
			sendDebug(); break;
			
		case 'b':
			sendYaw(); break;
		case 'B':
			receiveYaw(); break;
		}
	}
	while( timeout > millis() );	
}