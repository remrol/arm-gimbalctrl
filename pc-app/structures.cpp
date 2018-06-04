#include "stdafx.h"
#include "structures.h"


std::vector<std::string> tokenize( const std::string& _str)
{
	std::vector< std::string > tokens;
	boost::split( tokens, _str, boost::is_any_of( std::string(", ")));
  return tokens;
}

bool Config::fromString( const std::string& _data )
{
	std::vector< std::string > tokens = tokenize( _data );
	if( tokens.size() != 19 )
	{
		return false;
	}

	pulse_min						= atoi( tokens[0].c_str() );
	pulse_max						= atoi( tokens[1].c_str() );
	pulse_dband_lo					= atoi( tokens[2].c_str() );
	pulse_dband_hi					= atoi( tokens[3].c_str() );
	pwm_scale_factor				= atoi( tokens[4].c_str() );
	speed_smooth_factor				= atoi( tokens[5].c_str() );
	yaw_speed_smooth_factor         = atoi( tokens[6].c_str() );
	power							= atoi( tokens[7].c_str() );
	expo_percent					= atoi( tokens[8].c_str() );
	process_pulse_interval_ms		= atoi( tokens[9].c_str() );
	process_speedsmooth_interval_ms	= atoi( tokens[10].c_str() );
	mot_stop_nopulse_timeout_ms		= atoi( tokens[11].c_str() );
	mot_disable_stopped_timeout_ms	= atoi( tokens[12].c_str() );
	storm32_update_inteval_ms       = atoi( tokens[13].c_str() );
	yawPID_p						= atof( tokens[14].c_str());
	yawPID_i						= atof( tokens[15].c_str());
	yawPID_d						= atof( tokens[16].c_str());
	yawMaxSpeed						= atoi( tokens[17].c_str() );
	crc								= atoi( tokens[18].c_str() );

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
		"PWM scale factor " << pwm_scale_factor << "\r\n" <<
		"Speed smooth factor " << speed_smooth_factor << "\r\n" <<
		"Yaw speed smooth factor " << yaw_speed_smooth_factor << "\r\n" <<
		"Power " << power << "\r\n" <<	
		"Expo " << expo_percent << "\r\n" <<	
		"Process pulse interval " << process_pulse_interval_ms << " [ms]\r\n" <<	
		"Process speed smooth interval " << process_speedsmooth_interval_ms << " [ms]\r\n" << 
		"Motor stop if no pulse timeout " << mot_stop_nopulse_timeout_ms << " [ms]\r\n" << 
		"Motor disable if no pulse timeout " << mot_disable_stopped_timeout_ms << " [ms]\r\n" << 
		"Storm32 update interval " << storm32_update_inteval_ms << " [ms]\r\n" << 
		"yaw PID   P " << yawPID_p << "   I " << yawPID_i << "   D " << yawPID_p << "\r\n" << 
		"yaw max speed " << yawMaxSpeed << "\r\n" <<
		"CRC8 " << crc;	

  return ss.str();
}

bool State::fromString( const std::string& _data )
{
	std::vector< std::string > tokens = tokenize( _data );
	if( tokens.size() != 10 )
	{
		return false;
	}

	timeStampSec        = atoi( tokens[0].c_str() ) / 1000.0;
	motorPosition       = atoi( tokens[1].c_str() );
	lastPulse1TimeSec   = atoi( tokens[2].c_str() ) / 1000.0;
	lastPulse3TimeSec   = atoi( tokens[3].c_str() ) / 1000.0;
	pulse1Duration      = atoi( tokens[4].c_str() );
	pulse3Duration      = atoi( tokens[5].c_str() );
	motorDirection      = atoi( tokens[6].c_str() );
	motorSpeed          = atoi( tokens[7].c_str() );
	speed               = atoi( tokens[8].c_str() );
	ocr0                = atoi( tokens[9].c_str() );

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
		"Pulse1 duration, timestamp " << pulse1Duration << "," << lastPulse1TimeSec << "\r\n" <<	
		"Pulse3 duration, timestamp " << pulse3Duration << "," << lastPulse3TimeSec  << "\r\n" <<	
		"Speed " << speed << "\r\n" <<	
		"OCR0 " << ocr0;

  return ss.str();
}
