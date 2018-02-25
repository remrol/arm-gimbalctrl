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
	
	// crc
	uint8_t crc;	
};

struct State
{
	// Motor position
	volatile uint16_t motorPosition; // =0
	
	// Timestamp of last measured pulse
	volatile uint32_t lastPulseTime; // = 0;
	
	volatile uint16_t pulseDuration; // = 0;	// Last measured pulse duration in ms/2, 0 if not available.	
	
	
	volatile int8_t actualDirection; // = 0;
	
	volatile int8_t actualSpeed; // = 0;	
	
	volatile int8_t speed; // = 0;	
	
	volatile uint32_t timer1OverflowCount; // = 0;				// Timer 1 overflow count.	
};

void stateInit();

void configEepromLoad();
void configEepromSave();

void configLoadDefaults();

extern Config g_config;
extern State  g_state;


#endif

