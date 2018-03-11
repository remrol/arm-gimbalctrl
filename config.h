#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

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

	uint16_t mot_stop_nopulse_timeout_ms;
	uint16_t mot_disable_stopped_timeout_ms;
	
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

	uint16_t pulseDuration;	// Last measured pulse duration in ms, 0 if not available.
	int16_t  speed;
	int8_t	 motorDirection;
	int16_t  motorSpeed;
	uint8_t  ocr0;

	int16_t  diag0;
	int16_t  diag1;
	
	uint32_t baroTimeStamp;
	int32_t  baroPressure;
	int16_t  baroTempX10;
	
	uint32_t magnTimeStamp;
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

void stateInit();

void configEepromLoad();
void configEepromSave();

void configLoadDefaults();

void diagMinMax(int16_t value);
void diagReset();

extern Config g_config;
extern State  g_state;


#endif

