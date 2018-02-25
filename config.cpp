#include "config.h"
#include "servo.h"
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
	g_config.pulse_min = 950 / PULSE_DURATION_SCALE;
	g_config.pulse_max = 2200 / PULSE_DURATION_SCALE;
	g_config.pulse_center = ( g_config.pulse_min + g_config.pulse_max ) / 2;
	g_config.pulse_center_lo = g_config.pulse_center - 300 / 2;
	g_config.pulse_center_hi = g_config.pulse_center + 300 / 2;
	g_config.power = 128;
	g_config.expo_percent = 0;
	
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
	g_state.motorPosition = 0;
	g_state.lastPulseTime = 0;		
	g_state.pulseDuration = 0;
	g_state.actualDirection = 0;
	g_state.actualSpeed = 0;
	g_state.speed = 0;
	g_state.timer1OverflowCount = 0;
}
