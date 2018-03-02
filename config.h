#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#define MOT_STOP_NO_PULSE_TIMEOUT_MS (2*1000)
#define MOT_DISABLE_TIMEOUT_MS (10*1000)

struct Config
{
	uint16_t pulse_min;
	uint16_t pulse_max;
	uint16_t pulse_dband_lo;
	uint16_t pulse_dband_hi;
	
	uint16_t pwm_scale_factor;
	int8_t   speed_smooth_factor;
	uint8_t  power; // 1..128
	uint8_t  expo_percent;
	
	uint16_t process_pulse_interval_ms;
	uint16_t process_speedsmooth_interval_ms;
	
	// crc
	uint8_t crc;	
};

struct State
{
	volatile uint32_t vT1OverflowCount; // Timer 1 overflow count.
	volatile uint16_t vMotorPosition;	// Motor position

	volatile uint32_t vPulseTimeStamp;	// Timestamp of last measured pulse
	volatile uint32_t vPulseDurationSum;
	volatile uint16_t vPulseDurationSumCount;

	uint16_t pulseDuration;	// Last measured pulse duration in ms/2, 0 if not available.
	int16_t  speed;
	int8_t	 motorDirection;
	int16_t  motorSpeed;
	uint8_t  ocr0;
};

void stateInit();

void configEepromLoad();
void configEepromSave();

void configLoadDefaults();

extern Config g_config;
extern State  g_state;


#endif

