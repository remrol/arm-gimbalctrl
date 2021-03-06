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
	uint32_t baroEventWorkTimestamp;
	uint8_t  baroEventType;
	int16_t  baroTemperature;
	int32_t  baroPressure;
	
	uint32_t magnEventTimestamp;
	int16_t  magnX;
	int16_t  magnY;
	int16_t  magnZ;
	
	uint32_t mpuEventTimeStamp;
	int16_t  mpuAccelX;
	int16_t  mpuAccelY;
	int16_t  mpuAccelZ;
	int16_t  mpuGyroX;
	int16_t  mpuGyroY;
	int16_t  mpuGyroZ;
};

extern SensorsState g_sensorsState;

#endif // SENSORS_H


