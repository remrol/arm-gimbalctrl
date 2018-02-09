#include "calculations.h"
#include "servo.h"

uint16_t expo (uint16_t pulse_time_ms, uint8_t percent /* 0-100 */) 
{
	#define X_RANGE 512L
	#define P_RANGE 100L

	// scale down to -512/512
	int32_t x = ((int32_t) pulse_time_ms) * X_RANGE / ( ( PULSE_MAX_MS - PULSE_MIN_MS ) / 2 ); 
	
	// all calculations from this point on are signed 10 bits:
	x = ( x * x * x / 65536 * percent / ( X_RANGE * X_RANGE / 65536 ) + ( P_RANGE - percent ) * x + P_RANGE / 2 ) / P_RANGE;
	
	// now scale it back up to full servo pulse:
	x = x * ( ( PULSE_MAX_MS - PULSE_MIN_MS ) / 2 ) / X_RANGE;
	
	return (uint16_t) x;
}

void mix (int proportional, int differential, int* a, int* b) 
{
	*a = (proportional+differential)/2;
	*b = (proportional-differential)/2;
}

void slowdown (int input, int* output, int increment) 
{
	if (increment > 0) 
	{
		if (*output	< input) 
		{
			*output += increment;
		}
	}
	else 
	{
		if (*output > input) 
		{
			*output += increment;
		}
	}
}

