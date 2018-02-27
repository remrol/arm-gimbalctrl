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
