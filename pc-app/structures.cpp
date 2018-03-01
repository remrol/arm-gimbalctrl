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
	pulse_dband_lo    = atoi( tokens[2].c_str() );
	pulse_dband_hi    = atoi( tokens[3].c_str() );
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
		"Pulse dband lo " << pulse_dband_lo << "\r\n" <<	
		"Pulse dband hi " << pulse_dband_hi << "\r\n" <<	
		"Power " << power << "\r\n" <<	
		"Expo " << expo_percent << "\r\n" <<	
		"CRC8 " << crc;	

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

	timeStampSec        = atoi( tokens[0].c_str() ) / 1000.0;
	motorPosition       = atoi( tokens[1].c_str() );
	lastPulseTimeSec    = atoi( tokens[2].c_str() ) / 1000.0;
	pulseDuration       = atoi( tokens[3].c_str() );
	motorDirection      = atoi( tokens[4].c_str() );
	motorSpeed          = atoi( tokens[5].c_str() );
	speed               = atoi( tokens[6].c_str() );
	ocr0                = atoi( tokens[7].c_str() );

	return true;
}

std::string State::toDisplayableString() const
{
	std::stringstream ss;
	ss << 
		"Timestamp " << timeStampSec << "\r\n" <<	
		"Motor pos " << motorPosition << "\r\n" <<	
		"Motor direction " << motorDirection << "\r\n" <<	
		"Motor speed " << motorSpeed << "\r\n" <<	
		"Pulse duration " << pulseDuration << "\r\n" <<	
		"Pulse timestamp " << lastPulseTimeSec << "\r\n" <<	
		"Speed " << speed << "\r\n" <<	
		"OCR0 " << ocr0;

  return ss.str();
}
