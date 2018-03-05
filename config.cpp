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
	g_config.speed_smooth_factor = 2;
	g_config.power = 128;
	g_config.expo_percent = 0;
	
	g_config.process_pulse_interval_ms = 100;
	g_config.process_speedsmooth_interval_ms = 10;
	
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
	g_state.vPulseTimeStamp = 0;		
	g_state.pulseDuration = 0;
	g_state.vPulseDurationSum = 0;
	g_state.vPulseDurationSumCount = 0;
	g_state.speed = 0;
	g_state.motorDirection = 0;
	g_state.motorSpeed = 0;
	g_state.vT1OverflowCount = 0;
	g_state.ocr0 = 0;
	g_state.diag0 = INT16_MIN;
	g_state.diag1 = INT16_MIN;
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

