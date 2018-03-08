#ifndef HMC5883L_H
#define HMC5883L_H

#include <inttypes.h>

void HMC5883L_init();
void HMC5883L_read();

struct StateHMC5883L
{
	int16_t magx;
	int16_t magy;
	int16_t magz;
	uint32_t timeStamp;
};

extern StateHMC5883L g_stateHMC5883L;

#endif // HMC5883L_H
