#pragma once

struct Config
{
	int pulse_min;
	int pulse_max;
	int pulse_dband_lo;
	int pulse_dband_hi;
	int pwm_scale_factor;
	int speed_smooth_factor;
	int yaw_speed_smooth_factor;
	int power; // 1..128
	int expo_percent;
	int process_pulse_interval_ms;
	int process_speedsmooth_interval_ms;
	int mot_stop_nopulse_timeout_ms;
	int mot_disable_stopped_timeout_ms;
	int storm32_update_inteval_ms;
	double yawPID_p;
	double yawPID_i;
	double yawPID_d;
	int yawMaxSpeed;
	int sensors_update_interval_ms;
	int crc;	// crc

	bool fromString( const std::string& _data );
	std::string toDisplayableString() const;
};

struct State
{
	double timeStampSec;
	int motorPosition;	// Motor position
	double lastPulse1TimeSec;	// Timestamp of last measured pulse
	double lastPulse3TimeSec;	// Timestamp of last measured pulse
	int pulse1Duration;	// Last measured pulse duration in ms/2, 0 if not available.	
	int pulse3Duration;	// Last measured pulse duration in ms/2, 0 if not available.	
	int	motorDirection;
	int motorSpeed;
	int speed;
	int ocr0;

	bool fromString( const std::string& _data );
	std::string toDisplayableString() const;
};

std::vector<std::string> tokenize( const std::string& _str);
