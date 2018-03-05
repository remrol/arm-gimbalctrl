#include "stdafx.h"
#include "Device.h"


Device::Device()
{
}

bool Device::isOpened()
{
	return m_serial.IsOpened() ? true : false;
}

void Device::close()
{
	m_serial.Close();
}

bool Device::open( int _port, int _baudRate, std::string& _message )
{
	// Open serial
	if( !m_serial.Open( _port, _baudRate ) )
	{
		_message = "COM port not opened";
		return false;
	}

	// Validate device, close serial if validation failed
	if( !validateDevice( _message ) )
	{
		m_serial.Close();
		return false;
	}

	return true;
}

bool Device::validateDevice(std::string& _status)
{
	// Not valid if no connection 
	if( !isOpened() )
	{
		_status = "COM port not opened";
		return false;
	}

	// Get device info.
	std::string msg = sendReceive( "i" );
	// First word must be 'protocol'
	if( msg.find( "Gimbal" ) == std::string::npos)
	{
		_status = std::string( "Unexpected device info msg " ) + msg;
		return false;
	}

	_status = std::string( "OK " ) + msg;
	return true;
}

std::string Device::sendReceive( const std::string& _msg)
{
	if(!isOpened())
		return std::string();

	m_serial.SendData( _msg.data(), (int) _msg.size());

	char character[1024];
	std::string recv;
	int waitCount = 0;

	while( true )
	{
		int received = m_serial.ReadData(character, 1);

		assert( received == 0 || received == 1 );

		if( received == 0 )
		{
			::Sleep(1);
			++waitCount;

			if( waitCount > 500 )
			{
				return std::string("");
			}
			continue;
		}
		else if( character[0] == 13 )
		{
			continue;
		}
		else if( character[0] == 10 )
		{
			break;
		}
		else
		{
			recv += character[0];
		}
	}

	return recv;
}

bool Device::getConfig( Config& _config )
{
	if( !isOpened())
	{
		L_ << "getConfig, not connected";
		return false;
	}

	std::string msg = sendReceive("c");
	if( !_config.fromString( msg ) )
	{
		L_ << "getConfig, cannot parse message " << msg;
		return false;
	}

	return true;
}

bool Device::getState( State& _state )
{
	if( !isOpened())
	{
		L_ << "getState, not connected";
		return false;
	}

	std::string msg = sendReceive("s");
	if( !_state.fromString( msg ) )
	{
		L_ << "getState, cannot parse message " << msg;
		return false;
	}

	return true;
}


bool Device::getServoRange( int& _min, int& _dbandLo, int& _dbandHi, int& _max)
{
	if( !isOpened())
	{
		L_ << "getServoRange, not connected";
		return false;
	}

	std::string msg = sendReceive("l");
	std::vector< std::string > tokens;
	boost::split( tokens, msg, boost::is_any_of( std::string(", ")));
	if( tokens.size() != 4 )
	{
		L_ << "getServoRange, cannot parse msg";
		return false;
	}

	_min = atoi( tokens[0].c_str() );
	_max = atoi( tokens[1].c_str() );
	_dbandLo = atoi( tokens[2].c_str() );
	_dbandHi = atoi( tokens[3].c_str() );

  return true;
}

bool Device::setServoRange( int _min, int _dbandLo, int _dbandHi, int _max)
{
	if( !isOpened())
	{
		L_ << "setServoRange, not connected";
		return false;
	}

  std::stringstream ss;
  ss << "L " << _min << " " << _max << " " << _dbandLo << " " << _dbandHi << "\r\n";

  std::string msg = sendReceive( ss.str());
  L_ << msg;
  return true;
}

bool Device::configSaveToEeprom()
{
	if( !isOpened())
	{
		L_ << "configSaveToEeprom, not connected";
		return false;
	}

	std::string msg = sendReceive("W");
	if( msg.empty() )
	{
		L_ << "configSaveToEeprom, unexpected status " << msg;
		return false;
	}

	return true;
}

bool Device::getDiagnostics( int& _diag0, int& _diag1 )
{
	if( !isOpened())
	{
		L_ << "getDiagnostics, not connected";
		return false;
	}

	std::string msg = sendReceive("b");
	std::vector< std::string > tokens;
	boost::split( tokens, msg, boost::is_any_of( std::string(", ")));
	if( tokens.size() != 2 )
	{
		L_ << "getDiagnostics, cannot parse msg";
		return false;
	}

	_diag0 = atoi( tokens[0].c_str() );
	_diag1 = atoi( tokens[1].c_str() );

  return true;
}

bool Device::configLoadDefaults()
{
	if( !isOpened())
	{
		L_ << "configLoadDefaults, not connected";
		return false;
	}

	std::string msg = sendReceive("d");
  if( msg.empty() || msg[0] != '1' )
  {
		L_ << "configLoadDefaults, unexpected status " << msg;
		return false;
  }

  return true;
}

bool Device::getExpo( int& _expo )
{
	if( !isOpened())
	{
		L_ << "getExpo, not connected";
		return false;
	}
	std::string msg = sendReceive("e");
	std::vector< std::string > tokens;
	boost::split( tokens, msg, boost::is_any_of( std::string(", ")));
	if( tokens.size() != 1 )
	{
		L_ << "setExpo, cannot parse msg";
		return false;
	}

  _expo = atoi( tokens[0].c_str());
  return true;
}

bool Device::setExpo( int _expo )
{
	if( !isOpened())
	{
		L_ << "setExpo, not connected";
		return false;
	}

  std::stringstream ss;
  ss << "E " << _expo << "\r\n";

  std::string msg = sendReceive( ss.str());
  L_ << msg;

  return true;
}

bool Device::getPower( int& _power )
{
	if( !isOpened())
	{
		L_ << "getPower, not connected";
		return false;
	}
	std::string msg = sendReceive("p");
	std::vector< std::string > tokens;
	boost::split( tokens, msg, boost::is_any_of( std::string(", ")));
	if( tokens.size() != 1 )
	{
		L_ << "getPower, cannot parse msg";
		return false;
	}

  _power = atoi( tokens[0].c_str());
  return true;
}

bool Device::setPower( int _power )
{
	if( !isOpened())
	{
		L_ << "setPower, not connected";
		return false;
	}

  std::stringstream ss;
  ss << "P " << _power << "\r\n";

  std::string msg = sendReceive( ss.str());
  L_ << msg;

  return true;
}

bool Device::getPwmScaleFactor( int& _scaleFactor )
{
	if( !isOpened())
	{
		L_ << "getPwmScaleFactor, not connected";
		return false;
	}
	std::string msg = sendReceive("f");
	std::vector< std::string > tokens;
	boost::split( tokens, msg, boost::is_any_of( std::string(", ")));
	if( tokens.size() != 1 )
	{
		L_ << "getPwmScaleFactor, cannot parse msg";
		return false;
	}

  _scaleFactor = atoi( tokens[0].c_str());
  return true;
}

bool Device::setPwmScaleFactor( int _scaleFactor )
{
	if( !isOpened())
	{
		L_ << "setPwmScaleFactor, not connected";
		return false;
	}

  std::stringstream ss;
  ss << "F " << _scaleFactor << "\r\n";

  std::string msg = sendReceive( ss.str());
  L_ << msg;

  return true;
}


bool Device::getProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms)
{
	if( !isOpened())
	{
		L_ << "getProcessIntervals, not connected";
		return false;
	}
	std::string msg = sendReceive("a");
	std::vector< std::string > tokens;
	boost::split( tokens, msg, boost::is_any_of( std::string(", ")));
	if( tokens.size() != 2 )
	{
		L_ << "getProcessIntervals, cannot parse msg";
		return false;
	}

  _process_pulse_interval_ms = atoi( tokens[0].c_str());
  _process_speedsmooth_interval_ms = atoi( tokens[1].c_str());
  return true;
}

bool Device::setProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms)
{
	if( !isOpened())
	{
		L_ << "setProcessIntervals, not connected";
		return false;
	}

  std::stringstream ss;
  ss << "A " << _process_pulse_interval_ms << " " << _process_speedsmooth_interval_ms << "\r\n";

  std::string msg = sendReceive( ss.str());
  L_ << msg;

  return true;
}

