#pragma once

struct Config
{
	int pulse_min;
	int pulse_max;
	int pulse_center_lo;
	int pulse_center_hi;
	int power; // 1..128
	int expo_percent;
	int crc;	// crc

	bool fromString( const std::string& _data );
	std::string toDisplayableString() const;
};

struct State
{
	int motorPosition;	// Motor position
	int lastPulseTimeMs;	// Timestamp of last measured pulse
	int pulseDuration;	// Last measured pulse duration in ms/2, 0 if not available.	
	int	actualDirection;
	int actualSpeed;
	int speed;
	int timer1OverflowCount; // Timer 1 overflow count.		
	int ocr0;

	bool fromString( const std::string& _data );
	std::string toDisplayableString() const;
};

