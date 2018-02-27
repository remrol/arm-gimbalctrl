#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#define PULSE_DURATION_SCALE 2

struct Config
{
	uint16_t pulse_min;
	uint16_t pulse_max;
	uint16_t pulse_dband_lo;
	uint16_t pulse_dband_hi;
	
	uint8_t  power; // 1..128
	uint8_t  expo_percent;
	
	// crc
	uint8_t crc;	
};

struct State
{
	volatile uint16_t motorPosition;	// Motor position
	volatile uint32_t lastPulseTime;	// Timestamp of last measured pulse
	volatile uint16_t pulseDuration;	// Last measured pulse duration in ms/2, 0 if not available.	
	volatile int8_t	  actualDirection;
	volatile int8_t   actualSpeed;
	volatile int8_t   speed;
	volatile uint32_t timer1OverflowCount; // Timer 1 overflow count.		
	volatile uint8_t  ocr0;
};

void stateInit();

void configEepromLoad();
void configEepromSave();

void configLoadDefaults();

extern Config g_config;
extern State  g_state;


#endif

