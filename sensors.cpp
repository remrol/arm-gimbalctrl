#define __STDC_LIMIT_MACROS
#include "sensors.h"
#include "i2cdevlib/I2Cdev.h"
#include "i2cdevlib/HMC5883L.h"
#include "i2cdevlib/BMP085.h"
#include "i2cdevlib/MPU6050.h"
#include "time.h"
#include "config.h"

SensorsState g_sensorsState;

HMC5883L g_magn;
BMP085   g_baro;
MPU6050	 g_mpu6050;


void sensorsInit()
{
	g_sensorsState.initialized = false;
	
	Fastwire::setup(400, true);
	
	g_magn.initialize();
	g_baro.initialize();
	g_mpu6050.initialize();	
	
	if( g_magn.testConnection() && g_baro.testConnection() && g_mpu6050.testConnection())
	{
		g_sensorsState.initialized = true;
		
		g_sensorsState.baroEventTimestamp = millis() + 100;
		g_sensorsState.baroEventType = BARO_EVENT_TEMP_TRIGGER;
		g_sensorsState.baroTemperature = INT16_MIN;
		g_sensorsState.baroPressure = INT32_MIN;
	}
}

void sensorsRead()
{
	if( !g_sensorsState.initialized)
		return;
		
	if( millis() >= g_sensorsState.baroEventTimestamp )
	{
		switch( g_sensorsState.baroEventType)
		{
		case BARO_EVENT_TEMP_TRIGGER:
			g_baro.setControl(BMP085_MODE_TEMPERATURE);
			g_sensorsState.baroEventTimestamp = millis() + g_baro.getMeasureDelayMilliseconds();
			g_sensorsState.baroEventType = BARO_EVENT_TEMP_WAIT;
			break;
			
		case BARO_EVENT_TEMP_WAIT:
			g_sensorsState.baroTemperature = g_baro.getTemperatureC_x10();
			g_baro.setControl(BMP085_MODE_PRESSURE_3);
			g_sensorsState.baroEventTimestamp = millis() + g_baro.getMeasureDelayMilliseconds();
			g_sensorsState.baroEventType = BARO_EVENT_PRESSURE_WAIT;
			break;
			
		case BARO_EVENT_PRESSURE_WAIT:
			g_sensorsState.baroPressure = g_baro.getPressure();
			g_state.baroTimeStamp = millis();
			g_state.baroTempX10 = g_sensorsState.baroTemperature;
			g_state.baroPressure = g_sensorsState.baroPressure;
			
			g_sensorsState.baroEventType = BARO_EVENT_TEMP_TRIGGER;
			g_sensorsState.baroEventTimestamp = millis() + 100;			// TODO: figure out more precise event time stamp
			break;			
		}
	}
	
		
		//---------------------------
		/*
		bpm.setControl(BMP085_MODE_TEMPERATURE);
		while (micros() - lastMicros < bpm.getMeasureDelayMicroseconds());
		g_state.diag0 = bpm.getTemperatureC() * 100;
		bpm.setControl(BMP085_MODE_PRESSURE_3);
		while (micros() - lastMicros < bpm.getMeasureDelayMicroseconds());
		g_state.diag1 = 10 * bpm.getAltitude(bpm.getPressure());
		*/
		//---------------------------
	
}
