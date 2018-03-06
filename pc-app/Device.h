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

  bool checkConnected( const std::string& _signature );
  bool checkExpectedTokensCount( const std::vector< std::string >& _tokens, int _tokensCount, const std::string& _signature );
  bool checkStatus( const std::string& _status, const std::string& _signature );
  bool checkStatus( const std::string& _status, int _v0, const std::string& _signature );
  bool checkStatus( const std::string& _status, int _v0, int _v1, const std::string& _signature );
  bool checkStatus( const std::string& _status, int _v0, int _v1, int _v2, int _v3, const std::string& _signature );

  static std::string buildMessage( char cmd );
  static std::string buildMessage( char cmd, int _arg0 );
  static std::string buildMessage( char cmd, int _arg0, int _arg1 );
  static std::string buildMessage( char cmd, int _arg0, int _arg1, int _arg2, int _arg3 );

	bool validateDevice(std::string& _status);
	std::string sendReceive( const std::string& _msg);

	CSerial m_serial;
};

