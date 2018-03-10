#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>


void sensorsInit();

void sensorsRead();

struct SensorsState
{
	uint8_t initialized;
};

extern SensorsState g_sensorsState;

#endif // SENSORS_H


