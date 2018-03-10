#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>


void sensorsInit();

void sensorsRead();

#define BARO_EVENT_TEMP_TRIGGER 0
#define BARO_EVENT_TEMP_WAIT 1
#define BARO_EVENT_PRESSURE_WAIT 2

struct SensorsState
{
	uint8_t initialized;
	
	uint32_t baroEventTimestamp;
	uint8_t  baroEventType;
	int16_t  baroTemperature;
	int32_t  baroPressure;
};

extern SensorsState g_sensorsState;

#endif // SENSORS_H


