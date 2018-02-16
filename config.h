#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

struct Config
{
	uint16_t pulse_min;
	uint16_t pulse_center;
	uint16_t pulse_max;
	uint16_t pulse_center_lo;
	uint16_t pulse_center_hi;
	
	uint8_t  power; // 1..128
	uint8_t  expo_percent;
};


void configLoad();

extern Config g_config;


#endif

