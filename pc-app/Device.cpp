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

bool Device::checkConnected( const std::string& _signature )
{
	if( !isOpened())
	{
    L_ << _signature << ", no connection";
		return false; 
	}

  return true;
}

bool Device::checkExpectedTokensCount( const std::vector< std::string >& _tokens, int _tokensCount, const std::string& _signature )
{
  if(_tokens.size() != _tokensCount)
  {
    std::string ss;
    for( size_t i = 0; i < _tokens.size(); ++i)
      ss += _tokens[i] + std::string(",");

    L_ << _signature << ", unexpected tokens count " << _tokensCount << " tokens:" << ss;
    return false;
  }

  return true;
}

bool isError(const std::string& _token)
{
  if( _token == ""  || _token.substr(3) == "ERR" )
    return true;

  return false;
}

bool Device::checkStatus( const std::string& _status, const std::string& _signature )
{
  std::vector<std::string> tokens = tokenize( _status );
  if( tokens.empty())
  {
    L_ << _signature << " empty status";
    return false;
  }

  if( isError(tokens[0] ) )
  {
    L_ << _signature << " error: " << _status;
    return false;
  }

  return true;
}

bool Device::checkStatus( const std::string& _status, int _v0, const std::string& _signature )
{
  std::vector<std::string> tokens = tokenize(_status);
  if( tokens.empty())
  {
    L_ << _signature << " empty status";
    return false;
  }

  if( isError(tokens[0] ) )
  {
    L_ << _signature << " error: " << _status;
    return false;
  }

  if( tokens.size() < 1 )
  {
    L_ << _signature << " not enough args " << _status;
    return false;
  }

  if(atoi(tokens[0].c_str()) != _v0)
  {
    L_ << _signature << " arg0 " << tokens[0] << " expected " << _v0 << " status: " << _status;
    return false;
  }

  return true;
}


bool Device::checkStatus( const std::string& _status, int _v0, int _v1, const std::string& _signature )
{
  std::vector<std::string> tokens = tokenize(_status);
  if( tokens.empty())
  {
    L_ << _signature << " empty status";
    return false;
  }

  if( isError(tokens[0] ) )
  {
    L_ << _signature << " error: " << _status;
    return false;
  }

  if( tokens.size() < 2 )
  {
    L_ << _signature << " not enough args " << _status;
    return false;
  }

  if(atoi(tokens[0].c_str()) != _v0)
  {
    L_ << _signature << " arg0 " << tokens[0] << " expected " << _v0 << " status: " << _status;
    return false;
  }

  if(atoi(tokens[1].c_str()) != _v1)
  {
    L_ << _signature << " arg1 " << tokens[1] << " expected " << _v1 << " status: " << _status;
    return false;
  }

  return true;
}

bool Device::checkStatus( const std::string& _status, int _v0, int _v1, int _v2, int _v3, const std::string& _signature )
{
  std::vector<std::string> tokens = tokenize(_status);
  if( tokens.empty())
  {
    L_ << _signature << " empty status";
    return false;
  }

  if( isError(tokens[0] ) )
  {
    L_ << _signature << " error: " << _status;
    return false;
  }

  if( tokens.size() < 4 )
  {
    L_ << _signature << " not enough args " << _status;
    return false;
  }

  if(atoi(tokens[0].c_str()) != _v0)
  {
    L_ << _signature << " arg0 " << tokens[0] << " expected " << _v0 << " status: " << _status;
    return false;
  }

  if(atoi(tokens[1].c_str()) != _v1)
  {
    L_ << _signature << " arg1 " << tokens[1] << " expected " << _v1 << " status: " << _status;
    return false;
  }

  if(atoi(tokens[2].c_str()) != _v2)
  {
    L_ << _signature << " arg2 " << tokens[2] << " expected " << _v2 << " status: " << _status;
    return false;
  }

  if(atoi(tokens[3].c_str()) != _v3)
  {
    L_ << _signature << " arg3 " << tokens[3] << " expected " << _v3 << " status: " << _status;
    return false;
  }

  return true;
}



#define VALIDATE_STATUS_4ARG(status, v0, v1, v2, v3)  \
  std::vector<std::string> tokens = tokenize(status); \
  if( tokens.empty())                                 \
  {                                                   \
    L_ << __FUNCTION__ << " empty status";            \
    return false;                                     \
  }                                                   \
  if( isError(tokens[0] ) )                           \
  {                                                   \
    L_ << __FUNCTION__ << " error: " << status;       \
    return false;                                     \
  }                                                   \
  if( tokens.size() < 4 )                             \
  {                                                   \
    L_ << __FUNCTION__ << " not enough args " << status; \
    return false;                                     \
  }                                                   \
  if(atoi(tokens[0].c_str()) != v0)                   \
  {                                                   \
    L_ << __FUNCTION__ << " arg0 " << tokens[0] << " expected " << v0 << " status: " << status; \
    return false;                                     \
  }                                                   \
  if(atoi(tokens[1].c_str()) != v1)                   \
  {                                                   \
    L_ << __FUNCTION__ << " arg1 " << tokens[1] << " expected " << v1 << " status: " << status; \
    return false;                                     \
  }                                                   \
  if(atoi(tokens[2].c_str()) != v2)                   \
  {                                                   \
    L_ << __FUNCTION__ << " arg2 " << tokens[2] << " expected " << v2 << " status: " << status; \
    return false;                                     \
  }                                                   \
  if(atoi(tokens[3].c_str()) != v3)                   \
  {                                                   \
    L_ << __FUNCTION__ << " arg3 " << tokens[3] << " expected " << v3 << " status: " << status; \
    return false;                                     \
  }


bool Device::validateDevice(std::string& _status)
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

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
  if( !checkConnected( __FUNCTION__ ) )
    return false;

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
  if( !checkConnected( __FUNCTION__ ) )
    return false;

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
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::vector< std::string > tokens = tokenize( sendReceive("l") );
  if( !checkExpectedTokensCount( tokens, 4, __FUNCTION__ ) )
    return false;

	_min = atoi( tokens[0].c_str() );
	_max = atoi( tokens[1].c_str() );
	_dbandLo = atoi( tokens[2].c_str() );
	_dbandHi = atoi( tokens[3].c_str() );

  return true;
}

bool Device::setServoRange( int _min, int _dbandLo, int _dbandHi, int _max)
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

  std::stringstream ss;
  ss << "L " << _min << " " << _max << " " << _dbandLo << " " << _dbandHi << "\r\n";

  std::string msg = sendReceive( ss.str());
  if( !checkStatus( msg, _min, _max, _dbandLo, _dbandHi, __FUNCTION__ ) )
    return false;

  return true;
}

bool Device::configSaveToEeprom()
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::string msg = sendReceive("W");
  if( !checkStatus( msg, __FUNCTION__ ) )
    return false;

	return true;
}

bool Device::getDiagnostics( int& _diag0, int& _diag1 )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::vector< std::string > tokens = tokenize( sendReceive("b") );
  if( !checkExpectedTokensCount( tokens, 2, __FUNCTION__ ) )
    return false;

	_diag0 = atoi( tokens[0].c_str() );
	_diag1 = atoi( tokens[1].c_str() );

  return true;
}

bool Device::configLoadDefaults()
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::string msg = sendReceive("d");
  if( !checkStatus( msg, __FUNCTION__ ) )
    return false;

  return true;
}

bool Device::getExpo( int& _expo )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::vector< std::string > tokens = tokenize( sendReceive("e") );
  if( !checkExpectedTokensCount( tokens, 1, __FUNCTION__ ) )
    return false;

  _expo = atoi( tokens[0].c_str());
  return true;
}

bool Device::setExpo( int _expo )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

  std::stringstream ss;
  ss << "E " << _expo << "\r\n";

  std::string msg = sendReceive( ss.str());
  if( !checkStatus( msg, _expo, __FUNCTION__ ) )
    return false;

  return true;
}

bool Device::getPower( int& _power )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::vector< std::string > tokens = tokenize( sendReceive("p") );
  if( !checkExpectedTokensCount( tokens, 1, __FUNCTION__ ) )
    return false;

  _power = atoi( tokens[0].c_str());
  return true;
}

bool Device::setPower( int _power )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

  std::stringstream ss;
  ss << "P " << _power << "\r\n";

  std::string msg = sendReceive( ss.str());
  if( !checkStatus( msg, _power, __FUNCTION__ ) )
    return false;

  return true;
}

bool Device::getPwmScaleFactor( int& _scaleFactor )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

	std::vector< std::string > tokens = tokenize( sendReceive("f") );
  if( !checkExpectedTokensCount( tokens, 1, __FUNCTION__ ) )
    return false;

  _scaleFactor = atoi( tokens[0].c_str());
  return true;
}

bool Device::setPwmScaleFactor( int _scaleFactor )
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

  std::stringstream ss;
  ss << "F " << _scaleFactor << "\r\n";

  std::string msg = sendReceive( ss.str());
  if( !checkStatus( msg, _scaleFactor, __FUNCTION__ ) )
    return false;

  return true;
}

bool Device::getProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms)
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

  std::string msg = sendReceive("a");
	std::vector< std::string > tokens = tokenize( msg );
  if( !checkExpectedTokensCount( tokens, 2, __FUNCTION__ ) )
    return false;

  _process_pulse_interval_ms = atoi( tokens[0].c_str());
  _process_speedsmooth_interval_ms = atoi( tokens[1].c_str());
  return true;
}

bool Device::setProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms)
{
  if( !checkConnected( __FUNCTION__ ) )
    return false;

  std::stringstream ss;
  ss << "A " << _process_pulse_interval_ms << " " << _process_speedsmooth_interval_ms << "\r\n";

  std::string msg = sendReceive( ss.str());
  if( !checkStatus( msg, _process_pulse_interval_ms, _process_speedsmooth_interval_ms, __FUNCTION__ ) )
    return false;

  return true;
}

