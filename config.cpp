#define __STDC_LIMIT_MACROS
#include "config.h"
#include <avr/eeprom.h>
#include <util/crc16.h>

Config g_config;
State  g_state;

uint8_t configComputeCrc(void)
{
	uint8_t i, crc;

	crc = 0;
	for(i = 0; i < sizeof(struct Config) - 1; ++i )
	{
		crc = _crc8_ccitt_update( crc, ((uint8_t*) &g_config)[i]);
	}

	return crc;
}

void configLoadDefaults()
{
	g_config.pulse_min = 950;
	g_config.pulse_max = 2200;
	g_config.pulse_dband_lo = ( ( g_config.pulse_min + g_config.pulse_max ) / 2 ) - 300 / 2;
	g_config.pulse_dband_hi = ( ( g_config.pulse_min + g_config.pulse_max ) / 2 ) + 300 / 2;
	g_config.pwm_scale_factor = 1200;
	g_config.speed_smooth_factor = 5;
	g_config.power = 128;
	g_config.expo_percent = 40;
	
	g_config.process_pulse_interval_ms = 100;
	g_config.process_speedsmooth_interval_ms = 20;

	g_config.mot_stop_nopulse_timeout_ms = 2*1000;
	g_config.mot_disable_stopped_timeout_ms = 10*1000;
	
	g_config.storm32_update_inteval_ms = 100;
	
	// Update crc
	g_config.crc = configComputeCrc();	
}

void configEepromLoad()
{
	// Read memory block.
	// config block address is 1
	eeprom_read_block( &g_config, (void*) 0, sizeof(struct Config));

	// Compare with read crc, if match then return..
	if(configComputeCrc() == g_config.crc)
		return;

	// .. otherwise fill with some default values.
	configLoadDefaults();
	configEepromSave();
}


void configEepromSave()
{
	// Update crc
	g_config.crc = configComputeCrc();

	// Save block.
	// config block address is 1
	eeprom_busy_wait();
	eeprom_update_block( &g_config, (void*) 0, sizeof(struct Config));
}


void stateInit()
{
	g_state.vMotorPosition = 0;
	g_state.vPulse1TimeStamp = 0;		
	g_state.pulse1Duration = 0;
	g_state.pulse3Duration = 0;
	g_state.vPulse1DurationSum = 0;
	g_state.vPulse1DurationSumCount = 0;
	g_state.vPulse3DurationSum = 0;
	g_state.vPulse3DurationSumCount = 0;
	g_state.vPulse3TimeStamp = 0;
	g_state.yawCtrlSpeed = 0;
	g_state.motorDirection = 0;
	g_state.motorSpeed = 0;
	g_state.vT1OverflowCount = 0;
	g_state.vT3OverflowCount = 0;
	g_state.ocr0 = 0;
	g_state.diag0 = INT16_MIN;
	g_state.diag1 = INT16_MIN;
	
	g_state.baroTimeStamp = 0;
	g_state.baroPressure = INT32_MIN;
	g_state.baroTempX10 = INT16_MIN;
	
	g_state.magnTimeStamp = 0;
	g_state.magnX = INT16_MIN;
	g_state.magnY = INT16_MIN;
	g_state.magnZ = INT16_MIN;
	
	g_state.mpuEventTimeStamp = 0;
	g_state.mpuAccelX = INT16_MIN;
	g_state.mpuAccelY = INT16_MIN;
	g_state.mpuAccelZ = INT16_MIN;
	g_state.mpuGyroX = INT16_MIN;
	g_state.mpuGyroY = INT16_MIN;
	g_state.mpuGyroZ = INT16_MIN;
	
	g_state.yawOffset = 0;
	g_state.yawError = 0;
	g_state.yawStabilizeMode = 0;
	g_state.yawPIDInput = 0;
	g_state.yawPIDSetPoint = 0;
	g_state.yawPIDOutput = 0;
	
	g_state.storm32YawAngle = 0;
	g_state.storm32YawTimeStamp = 0;

}

void diagMinMax(int16_t value)
{
	if( g_state.diag0 == INT16_MIN || value < g_state.diag0)
		g_state.diag0 = value;

	if( g_state.diag1 == INT16_MIN || value > g_state.diag1)
		g_state.diag1 = value;
}

void diagReset()
{
	g_state.diag0 = INT16_MIN;
	g_state.diag1 = INT16_MIN;
}

