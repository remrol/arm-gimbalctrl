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

#define ENSURE_CONNECTED                     \
	if( !isOpened())                           \
	{                                          \
    L_ << __FUNCTION__ << ", no connection"; \
		return false;                            \
	} 

#define EXPECT_TOKENS_COUNT(tokens, count)      \
  if(tokens.size() != count)                    \
  {                                             \
    std::string ss;                             \
    for( size_t i = 0; i < tokens.size(); ++i)  \
      ss += tokens[i] + std::string(",");       \
    L_ << __FUNCTION__ << ", unexpected tokens count " << count << " tokens:" << ss;  \
    return false;                               \
  }

bool isError(const std::string& _token)
{
  if( _token == "" )
    return true;
  if( _token.substr(3) == "ERR" )
    return true;

  return false;
}

#define VALIDATE_STATUS_0ARG(status)                  \
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
  }

#define VALIDATE_STATUS_1ARG(status, v0)              \
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
  if(atoi(tokens[0].c_str()) != v0)                   \
  {                                                   \
    L_ << __FUNCTION__ << " arg0 " << tokens[0] << " expected " << v0 << " status: " << status; \
    return false;                                     \
  }

#define VALIDATE_STATUS_2ARG(status, v0, v1)          \
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
  if( tokens.size() < 2 )                             \
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
  ENSURE_CONNECTED;

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
  ENSURE_CONNECTED;

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
  ENSURE_CONNECTED;

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
  ENSURE_CONNECTED;

	std::vector< std::string > tokens = tokenize( sendReceive("l") );
  EXPECT_TOKENS_COUNT(tokens, 4);

	_min = atoi( tokens[0].c_str() );
	_max = atoi( tokens[1].c_str() );
	_dbandLo = atoi( tokens[2].c_str() );
	_dbandHi = atoi( tokens[3].c_str() );

  return true;
}

bool Device::setServoRange( int _min, int _dbandLo, int _dbandHi, int _max)
{
  ENSURE_CONNECTED;

  std::stringstream ss;
  ss << "L " << _min << " " << _max << " " << _dbandLo << " " << _dbandHi << "\r\n";

  std::string msg = sendReceive( ss.str());
  VALIDATE_STATUS_4ARG( msg, _min, _max, _dbandLo, _dbandHi);
  return true;
}

bool Device::configSaveToEeprom()
{
  ENSURE_CONNECTED;

	std::string msg = sendReceive("W");
  VALIDATE_STATUS_0ARG(msg);

	return true;
}

bool Device::getDiagnostics( int& _diag0, int& _diag1 )
{
  ENSURE_CONNECTED;

	std::vector< std::string > tokens = tokenize( sendReceive("b") );
  EXPECT_TOKENS_COUNT( tokens, 2);

	_diag0 = atoi( tokens[0].c_str() );
	_diag1 = atoi( tokens[1].c_str() );

  return true;
}

bool Device::configLoadDefaults()
{
  ENSURE_CONNECTED;

	std::string msg = sendReceive("d");
  VALIDATE_STATUS_0ARG(msg);

  return true;
}

bool Device::getExpo( int& _expo )
{
  ENSURE_CONNECTED;

	std::vector< std::string > tokens = tokenize( sendReceive("e") );
  EXPECT_TOKENS_COUNT( tokens, 1 );

  _expo = atoi( tokens[0].c_str());
  return true;
}

bool Device::setExpo( int _expo )
{
  ENSURE_CONNECTED;

  std::stringstream ss;
  ss << "E " << _expo << "\r\n";

  std::string msg = sendReceive( ss.str());
  VALIDATE_STATUS_1ARG( msg, _expo);

  return true;
}

bool Device::getPower( int& _power )
{
  ENSURE_CONNECTED;

	std::vector< std::string > tokens = tokenize( sendReceive("p") );
  EXPECT_TOKENS_COUNT( tokens, 1 );

  _power = atoi( tokens[0].c_str());
  return true;
}

bool Device::setPower( int _power )
{
  ENSURE_CONNECTED;

  std::stringstream ss;
  ss << "P " << _power << "\r\n";

  std::string msg = sendReceive( ss.str());
  VALIDATE_STATUS_1ARG( msg, _power );

  return true;
}

bool Device::getPwmScaleFactor( int& _scaleFactor )
{
  ENSURE_CONNECTED;

	std::vector< std::string > tokens = tokenize( sendReceive("f") );
  EXPECT_TOKENS_COUNT( tokens, 1 );

  _scaleFactor = atoi( tokens[0].c_str());
  return true;
}

bool Device::setPwmScaleFactor( int _scaleFactor )
{
  ENSURE_CONNECTED;

  std::stringstream ss;
  ss << "F " << _scaleFactor << "\r\n";

  std::string msg = sendReceive( ss.str());
  VALIDATE_STATUS_1ARG( msg, _scaleFactor );

  return true;
}

bool Device::getProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms)
{
  ENSURE_CONNECTED;

  std::string msg = sendReceive("a");
	std::vector< std::string > tokens = tokenize( msg );
  EXPECT_TOKENS_COUNT( tokens, 2 );

  _process_pulse_interval_ms = atoi( tokens[0].c_str());
  _process_speedsmooth_interval_ms = atoi( tokens[1].c_str());
  return true;
}

bool Device::setProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms)
{
  ENSURE_CONNECTED;

  std::stringstream ss;
  ss << "A " << _process_pulse_interval_ms << " " << _process_speedsmooth_interval_ms << "\r\n";

  std::string msg = sendReceive( ss.str());
  VALIDATE_STATUS_2ARG( msg, _process_pulse_interval_ms, _process_speedsmooth_interval_ms);

  return true;
}

