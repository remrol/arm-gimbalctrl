#pragma once

struct Config
{
	int pulse_min;
	int pulse_max;
	int pulse_dband_lo;
	int pulse_dband_hi;
	int pwm_scale_factor;
	int speed_smooth_factor;
	int power; // 1..128
	int expo_percent;
	int process_pulse_interval_ms;
	int process_speedsmooth_interval_ms;
	int crc;	// crc

	bool fromString( const std::string& _data );
	std::string toDisplayableString() const;
};

struct State
{
	double timeStampSec;
	int motorPosition;	// Motor position
	double lastPulseTimeSec;	// Timestamp of last measured pulse
	int pulseDuration;	// Last measured pulse duration in ms/2, 0 if not available.	
	int	motorDirection;
	int motorSpeed;
	int speed;
	int ocr0;

	bool fromString( const std::string& _data );
	std::string toDisplayableString() const;
};

std::vector<std::string> tokenize( const std::string& _str);
