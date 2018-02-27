#pragma once

#include "Serial.h"
#include "structures.h"

class Device
{
public:
	Device();

	bool isOpened();
	bool open( int _port, int _baudRate, std::string& _message );

	void close();

	bool getConfig( Config& _config );
  bool getState( State& _state );
  bool getServoRange( int& _min, int& _dbandLo, int& _dbandHi, int& _max);
  bool setServoRange( int _min, int _dbandLo, int _dbandHi, int _max);
  bool configSaveToEeprom();


private:

	bool validateDevice(std::string& _status);
	std::string sendReceive( const std::string& _msg);

	CSerial m_serial;
};

