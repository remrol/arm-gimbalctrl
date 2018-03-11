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

    bool getProcessing( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms, int& _speedSmoothRatio);
    bool setProcessing( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms, int _speedSmoothRatio);

    bool getMotorParams( int& _power, int& _scaleFactor, int& _expo );
    bool setMotorParams( int _power, int _scaleFactor, int _expo );

    bool getTimeouts( int& _mot_stop_nopulse_timeout_ms, int& _mot_disable_stopped_timeout_ms );
    bool setTimeouts( int _mot_stop_nopulse_timeout_ms, int _mot_disable_stopped_timeout_ms );

	bool getSensors( 
		double& _baroTimeStampSec, int& _baroTemperature, int& _baroPressure,
		double& _magnTimeStamp, int& _magnX, int& _magnY, int& _magnZ );

private:

    bool checkConnected( const std::string& _signature );
    bool checkExpectedTokensCount( const std::vector< std::string >& _tokens, int _tokensCount, const std::string& _signature );
    bool checkStatus( const std::string& _status, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, int _v1, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, int _v1, int _v2, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, int _v1, int _v2, int _v3, const std::string& _signature );

    static std::string buildMessage( char cmd );
    static std::string buildMessage( char cmd, int _arg0 );
    static std::string buildMessage( char cmd, int _arg0, int _arg1 );
    static std::string buildMessage( char cmd, int _arg0, int _arg1, int _arg2 );
    static std::string buildMessage( char cmd, int _arg0, int _arg1, int _arg2, int _arg3 );

	bool validateDevice(std::string& _status);
	std::string sendReceive( const std::string& _msg);

	CSerial m_serial;
};

