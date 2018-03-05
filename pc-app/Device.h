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

  bool configSaveToEeprom();
  bool configLoadDefaults();

  bool getServoRange( int& _min, int& _dbandLo, int& _dbandHi, int& _max);
  bool setServoRange( int _min, int _dbandLo, int _dbandHi, int _max);

  bool getDiagnostics( int& _diag0, int& _diag1 );

  bool getExpo( int& _expo );
  bool setExpo( int _expo );

  bool getPower( int& _power );
  bool setPower( int _power );

  bool getPwmScaleFactor( int& _scaleFactor );
  bool setPwmScaleFactor( int _scaleFactor );

  bool getProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms);
  bool setProcessIntervals( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms);

private:

	bool validateDevice(std::string& _status);
	std::string sendReceive( const std::string& _msg);

	CSerial m_serial;
};

