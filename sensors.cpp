#include "sensors.h"
#include "i2cdevlib/I2Cdev.h"
#include "i2cdevlib/HMC5883L.h"
#include "i2cdevlib/BMP085.h"
#include "i2cdevlib/MPU6050.h"
#include "time.h"

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
	}
}

void sensorsRead()
{
	if( !g_sensorsState.initialized)
		return;
		
		
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
