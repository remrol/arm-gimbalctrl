#include "stdafx.h"
#include "structures.h"

bool Config::fromString( const std::string& _data )
{
	std::vector< std::string > tokens;
	boost::split( tokens, _data, boost::is_any_of( std::string(", ")));

	if( tokens.size() != 7 )
	{
		return false;
	}

	pulse_min         = atoi( tokens[0].c_str() );
	pulse_max         = atoi( tokens[1].c_str() );
	pulse_center_lo   = atoi( tokens[2].c_str() );
	pulse_center_hi   = atoi( tokens[3].c_str() );
	power             = atoi( tokens[4].c_str() );
	expo_percent      = atoi( tokens[5].c_str() );
	crc               = atoi( tokens[6].c_str() );

	return true;
}

std::string Config::toDisplayableString() const
{
	std::stringstream ss;
	ss << 
		"Pulse min " << pulse_min << "\r\n" <<	
		"Pulse max " << pulse_max << "\r\n" <<	
		"Pulse center lo " << pulse_center_lo << "\r\n" <<	
		"Pulse center hi " << pulse_center_hi << "\r\n" <<	
		"Power " << power << "\r\n" <<	
		"Expo " << expo_percent << "\r\n" <<	
		"CRC8" << crc;	

  return ss.str();
}

bool State::fromString( const std::string& _data )
{
	std::vector< std::string > tokens;
	boost::split( tokens, _data, boost::is_any_of( std::string(", ")));

	if( tokens.size() != 8 )
	{
		return false;
	}

	motorPosition       = atoi( tokens[0].c_str() );
	lastPulseTimeMs     = atoi( tokens[1].c_str() );
	pulseDuration       = atoi( tokens[2].c_str() );
	actualDirection     = atoi( tokens[3].c_str() );
	actualSpeed         = atoi( tokens[4].c_str() );
	speed               = atoi( tokens[5].c_str() );
	timer1OverflowCount = atoi( tokens[6].c_str() );
  ocr0                = atoi( tokens[7].c_str() );

	return true;
}

std::string State::toDisplayableString() const
{
	std::stringstream ss;
	ss << 
		"Motor pos " << motorPosition << "\r\n" <<	
		"Last pulse time " << lastPulseTimeMs << "\r\n" <<	
		"Pulse duration " << pulseDuration << "\r\n" <<	
		"Actual direction " << actualDirection << "\r\n" <<	
		"Actual speed " << actualSpeed << "\r\n" <<	
		"Speed " << speed << "\r\n" <<	
		"Timer1 overflow cnt " << timer1OverflowCount << "\r\n" <<	
		"OCR0 pwm " << ocr0;

  return ss.str();
}
