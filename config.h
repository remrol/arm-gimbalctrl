#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
extern "C"
{
#include "PID_v1.h"
}

struct Config
{
	uint16_t pulse_min;
	uint16_t pulse_max;
	uint16_t pulse_dband_lo;
	uint16_t pulse_dband_hi;
	
	uint16_t pwm_scale_factor;
	int8_t   speed_normal_smooth_factor;
	int8_t   speed_yawstabilize_smooth_factor;
	uint8_t  power; // 1..128
	uint8_t  expo_percent;
	
	uint16_t process_pulse_interval_ms;
	uint16_t process_speedsmooth_interval_ms;

	uint16_t mot_stop_nopulse_timeout_ms;
	uint16_t mot_disable_stopped_timeout_ms;
	
	uint16_t storm32_update_inteval_ms;
	float    yawPID_p;
	float    yawPID_i;
	float    yawPID_d;
	int16_t  yawMaxSpeed;
	
	// crc
	uint8_t crc;	
};

struct State
{
	volatile uint32_t vT1OverflowCount; // Timer 1 overflow count.
	volatile uint32_t vT3OverflowCount; // Timer3 overflow count
	volatile uint16_t vMotorPosition;	// Motor position

	volatile uint32_t vPulse1TimeStamp;	// Timestamp of last measured pulse
	volatile uint32_t vPulse1DurationSum;
	volatile uint16_t vPulse1DurationSumCount;

	volatile uint32_t vPulse3TimeStamp;	// Timestamp of last measured pulse
	volatile uint32_t vPulse3DurationSum;
	volatile uint16_t vPulse3DurationSumCount;

	uint16_t pulse1Duration;	// Last measured pulse duration in ms, 0 if not available.
	uint16_t pulse3Duration;	// Last measured pulse duration in ms, 0 if not available.
	int16_t  yawCtrlSpeed;		// yaw user-control speed
	int8_t	 motorDirection;
	int16_t  motorSpeed;
	uint8_t  ocr0;

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
	
	int16_t  yawOffset;
	int16_t  yawPIDspeed;
	uint8_t  yawStabilizeMode;
	float	 yawPIDInput;
	float    yawPIDSetPoint;
	float    yawPIDOutput;
	PID      yawPID;
	
	int16_t  storm32YawAngle;
	uint32_t storm32YawTimeStamp;
};

void stateInit();

void configEepromLoad();
void configEepromSave();

void configLoadDefaults();

extern Config g_config;
extern State  g_state;


#endif

