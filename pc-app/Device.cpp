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
    if( _token == ""  || _token.substr(0, 3) == "ERR" )
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

bool Device::checkStatus( const std::string& _status, int _v0, int _v1, int _v2, const std::string& _signature )
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

    if( tokens.size() < 3 )
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


bool  Device::checkStatus( const std::string& _status, double _v0, double _v1, double _v2, int _v3, int _v4, int _v5, const std::string& _signature )
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

    if( tokens.size() < 6 )
    {
        L_ << _signature << " not enough args " << _status;
        return false;
    }

    if(atof(tokens[0].c_str()) != _v0)
    {
        L_ << _signature << " arg0 " << tokens[0] << " expected " << _v0 << " status: " << _status;
        return false;
    }

    if(atof(tokens[1].c_str()) != _v1)
    {
        L_ << _signature << " arg1 " << tokens[1] << " expected " << _v1 << " status: " << _status;
        return false;
    }

    if(atof(tokens[2].c_str()) != _v2)
    {
        L_ << _signature << " arg2 " << tokens[2] << " expected " << _v2 << " status: " << _status;
        return false;
    }

    if(atoi(tokens[3].c_str()) != _v3)
    {
        L_ << _signature << " arg3 " << tokens[3] << " expected " << _v3 << " status: " << _status;
        return false;
    }

    if(atoi(tokens[4].c_str()) != _v4)
    {
        L_ << _signature << " arg4 " << tokens[4] << " expected " << _v4 << " status: " << _status;
        return false;
    }

    if(atoi(tokens[5].c_str()) != _v5)
    {
        L_ << _signature << " arg5 " << tokens[5] << " expected " << _v5 << " status: " << _status;
        return false;
    }

    return true;
}


bool Device::validateDevice(std::string& _status)
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	// Get device info.
	std::string msg = sendReceive( buildMessage( 'i' ) );
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

	std::string msg = sendReceive( buildMessage( 'c' ) );
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

	std::string msg = sendReceive( buildMessage( 's' ) );
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

	std::vector< std::string > tokens = tokenize( sendReceive( buildMessage( 'l') ) );
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

    std::string msg = sendReceive( buildMessage( 'L', _min, _max, _dbandLo, _dbandHi ) );
    if( !checkStatus( msg, _min, _max, _dbandLo, _dbandHi, __FUNCTION__ ) )
        return false;

    return true;
}

bool Device::configSaveToEeprom()
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	std::string msg = sendReceive( buildMessage( 'W' ) );
    if( !checkStatus( msg, __FUNCTION__ ) )
        return false;

	return true;
}

bool Device::configLoadDefaults()
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	std::string msg = sendReceive( buildMessage( 'd' ) );
    if( !checkStatus( msg, __FUNCTION__ ) )
        return false;

    return true;
}

bool Device::getProcessing( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms, int& _speedSmoothRatio)
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'a' ) );
	std::vector< std::string > tokens = tokenize( msg );
    if( !checkExpectedTokensCount( tokens, 3, __FUNCTION__ ) )
        return false;

    _process_pulse_interval_ms = atoi( tokens[0].c_str());
    _process_speedsmooth_interval_ms = atoi( tokens[1].c_str());
    _speedSmoothRatio = atoi( tokens[2].c_str());

    return true;
}

bool Device::setProcessing( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms, int _speedSmoothRatio)
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'A', _process_pulse_interval_ms, _process_speedsmooth_interval_ms, _speedSmoothRatio ) );
    if( !checkStatus( msg, _process_pulse_interval_ms, _process_speedsmooth_interval_ms, _speedSmoothRatio, __FUNCTION__ ) )
        return false;

    return true;
}

bool Device::getMotorParams( int& _power, int& _scaleFactor, int& _expo )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'm' ) );
	std::vector< std::string > tokens = tokenize( msg );
    if( !checkExpectedTokensCount( tokens, 3, __FUNCTION__ ) )
        return false;

    _power = atoi( tokens[0].c_str());
    _scaleFactor = atoi( tokens[1].c_str());
    _expo = atoi( tokens[2].c_str());

    return true;
}

bool Device::setMotorParams( int _power, int _scaleFactor, int _expo )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'M', _power, _scaleFactor, _expo ) );
    if( !checkStatus( msg, _power, _scaleFactor, _expo, __FUNCTION__ ) )
        return false;

    return true;
}


bool Device::getTimeouts( int& _mot_stop_nopulse_timeout_ms, int& _mot_disable_stopped_timeout_ms )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;
    std::string msg = sendReceive( buildMessage( 't' ) );
	std::vector< std::string > tokens = tokenize( msg );
    if( !checkExpectedTokensCount( tokens, 2, __FUNCTION__ ) )
        return false;

    _mot_stop_nopulse_timeout_ms = atoi( tokens[0].c_str());
    _mot_disable_stopped_timeout_ms = atoi( tokens[1].c_str());

    return true;
}

bool Device::setTimeouts( int _mot_stop_nopulse_timeout_ms, int _mot_disable_stopped_timeout_ms )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'T', _mot_stop_nopulse_timeout_ms, _mot_disable_stopped_timeout_ms) );
    if( !checkStatus( msg, _mot_stop_nopulse_timeout_ms, _mot_disable_stopped_timeout_ms, __FUNCTION__ ) )
        return false;

    return true;
}


bool Device::getSensors( 
	double& _baroTimeStampSec, int& _baroTemperature, int& _baroPressure,
	double& _magnTimeStamp, int& _magnX, int& _magnY, int& _magnZ,
	double& _mpuTimeStamp, int& _accelX, int& _accelY, int& _accelZ, int& _gyroX, int& _gyroY, int& _gyroZ )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;
    std::string msg = sendReceive( buildMessage( 'e' ) );
	std::vector< std::string > tokens = tokenize( msg );
    if( !checkExpectedTokensCount( tokens, 14, __FUNCTION__ ) )
        return false;

    _baroTimeStampSec = atoi( tokens[0].c_str()) / 1000.0;
    _baroTemperature = atoi( tokens[1].c_str());
    _baroPressure = atoi( tokens[2].c_str());

	_magnTimeStamp = atoi( tokens[3].c_str()) / 1000.0;
	_magnX = atoi( tokens[4].c_str());
	_magnY = atoi( tokens[5].c_str());
	_magnZ = atoi( tokens[6].c_str());

	_mpuTimeStamp = atoi( tokens[7].c_str()) / 1000.0;
	_accelX = atoi( tokens[8].c_str());
	_accelY = atoi( tokens[9].c_str());
	_accelZ = atoi( tokens[10].c_str());
	_gyroX = atoi( tokens[11].c_str());
	_gyroY = atoi( tokens[12].c_str());
	_gyroZ = atoi( tokens[13].c_str());

    return true;
}


bool Device::readStorm32LiveData(double& _timeStamp)
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	std::vector< std::string > tokens = tokenize( sendReceive( buildMessage( 'f') ) );
    if( !checkExpectedTokensCount( tokens, 1, __FUNCTION__ ) )
        return false;

	_timeStamp = (double) atoi( tokens[0].c_str() ) / 1000.0;
	return true;
}


bool Device::getStorm32LiveData( int _offset, int& _data0, int& _data1, int& _data2 )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	std::vector< std::string > tokens = tokenize( sendReceive( buildMessage( 'g', _offset ) ) );
    if( !checkExpectedTokensCount( tokens, 3, __FUNCTION__ ) )
        return false;

	_data0 = atoi( tokens[0].c_str() );
	_data1 = atoi( tokens[1].c_str() );
	_data2 = atoi( tokens[2].c_str() );

	return true;
}

bool Device::getDebug( int _offset, int& _data )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	std::vector< std::string > tokens = tokenize( sendReceive( buildMessage( 'h', _offset ) ) );
    if( !checkExpectedTokensCount( tokens, 1, __FUNCTION__ ) )
        return false;

	_data = atoi( tokens[0].c_str() );

	return true;
}

bool Device::getYawConfig( 
	double& _pidP, double& _pidI, double& _pidD,
	int& _stabilizeSmoothFactor, int& _st32UpdateIntevalMs, int& _yawMaxSpeed )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'b' ) );
	std::vector< std::string > tokens = tokenize( msg );
    if( !checkExpectedTokensCount( tokens, 6, __FUNCTION__ ) )
        return false;

	_pidP = atof( tokens[0].c_str() );
	_pidI = atof( tokens[1].c_str() );
	_pidD = atof( tokens[2].c_str() );
	_stabilizeSmoothFactor = atoi(tokens[3].c_str());
	_st32UpdateIntevalMs = atoi(tokens[4].c_str());
	_yawMaxSpeed = atoi(tokens[5].c_str());

	return true;
}

bool Device::setYawConfig( 
	double _pidP, double _pidI, double _pidD,
	int _stabilizeSmoothFactor, int _st32UpdateIntevalMs, int _yawMaxSpeed )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

    std::string msg = sendReceive( buildMessage( 'B', _pidP, _pidI, _pidD, _stabilizeSmoothFactor, _st32UpdateIntevalMs, _yawMaxSpeed ) );
    if( !checkStatus( msg, _pidP, _pidI, _pidD, _stabilizeSmoothFactor, _st32UpdateIntevalMs, _yawMaxSpeed, __FUNCTION__ ) )
        return false;

    return true;

	return false;
}

	
bool Device::getStorm32( std::vector< int>& _values )
{
    if( !checkConnected( __FUNCTION__ ) )
        return false;

	_values.resize(32);

	if( !getStorm32LiveData( 0, _values[0], _values[1], _values[2]) )
		return false;
	if( !getStorm32LiveData( 3, _values[3], _values[4], _values[5]) )
		return false;
	if( !getStorm32LiveData( 6, _values[6], _values[7], _values[8]) )
		return false;
	if( !getStorm32LiveData( 9, _values[9], _values[10], _values[11]) )
		return false;
	if( !getStorm32LiveData( 12, _values[12], _values[13], _values[14]) )
		return false;
	if( !getStorm32LiveData( 15, _values[15], _values[16], _values[17]) )
		return false;
	if( !getStorm32LiveData( 18, _values[18], _values[19], _values[20]) )
		return false;
	if( !getStorm32LiveData( 21, _values[21], _values[22], _values[23]) )
		return false;
	if( !getStorm32LiveData( 24, _values[24], _values[25], _values[26]) )
		return false;
	if( !getStorm32LiveData( 27, _values[27], _values[28], _values[29]) )
		return false;
	if( !getStorm32LiveData( 29, _values[29], _values[30], _values[31]) )
		return false;

	return true;
}


