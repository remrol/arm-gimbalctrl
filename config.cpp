#include "config.h"
#include "servo.h"

Config g_config;


void configLoad()
{
	g_config.pulse_min = 950 / PULSE_DURATION_SCALE;
	g_config.pulse_max = 2200 / PULSE_DURATION_SCALE;
	g_config.pulse_center = ( g_config.pulse_min + g_config.pulse_max ) / 2;
	g_config.pulse_center_lo = g_config.pulse_center - 300 / 2;
	g_config.pulse_center_hi = g_config.pulse_center + 300 / 2;
	g_config.power = 128;
	g_config.expo_percent = 0;
}


