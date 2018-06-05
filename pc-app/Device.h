#pragma once

#include "Serial.h"
#include "structures.h"

class Device
{
public:

	enum Storm32DataOffset
	{
		ST32DO_STATE = 0,					// state
		ST32DO_status,						// status
		ST32DO_status2,						// status2
		ST32DO_status3,						// status3
		ST32DO_performanceMaxLoopDonex10,	// (s16)(Performance.MaxLoopdone*10); //performance
		ST32DO_imuErrorsCount,				// imu_ntbus_geterrocnt() + imu_onboard_geterrocnt(); //errors
		ST32DO_lipoVoltage,					// lipo_voltage(); //lipo_voltage;
		ST32DO_loopTimeMillis,				// (u16)looptime.millis_32; //timestamp
		ST32DO_cycleTime,					// (u16)(1.0E6f*FDT); //cycle time
		ST32DO_aImu1AnglePitch,				// (s16)(100*aImu1Angle.Pitch); //Imu1 angles, in 0.01°
		ST32DO_aImu1AngleRoll,				// (s16)(100*aImu1Angle.Roll);
		ST32DO_aImu1AngleYaw,				// (s16)(100*aImu1Angle.Yaw);
		ST32DO_Imu1AHRS_R_x,				// (s16)(10000*Imu1AHRS.R.x); //Imu1 R estimates, in 0.0001 g
		ST32DO_Imu1AHRS_R_y,				// (s16)(10000*Imu1AHRS.R.y);
		ST32DO_Imu1AHRS_R_z,				// (s16)(10000*Imu1AHRS.R.z);
		ST32DO_aImu1AnglePitch_minus_PID,	// (s16)(100*cPID[PITCH].Cntrl - aImu1Angle.Pitch); //relative PID output, in 0.01°
		ST32DO_aImu1AngleRoll_minus_PID,	// (s16)(100*cPID[ROLL].Cntrl - aImu1Angle.Roll);
		ST32DO_aImu1AngleYaw_minus_PID,		// (s16)(100*cPID[YAW].Cntrl - aImu1Angle.Yaw);
		ST32DO_InputSrcPitch,				// InputSrc.Pitch; //Rc Input values
		ST32DO_InputSrcRoll,				// InputSrc.Roll;
		ST32DO_InputSrcYaw,					// InputSrc.Yaw;
		ST32DO_aImu2AnglePitch,				// (s16)(100*aImu2Angle.Pitch); //Imu2 angles, in 0.01°
		ST32DO_aImu2AngleRoll,				// (s16)(100*aImu2Angle.Roll);
		ST32DO_aImu2AngleYaw,				// (s16)(100*aImu2Angle.Yaw);
		ST32DO_motorfoc_aEncoderAnglePitch,	// (s16)(100*motorfoc_aEncoderAngle(PITCH)); //Encoder angles, in 0.01°
		ST32DO_motorfoc_aEncoderAngle_Roll,	// (s16)(100*motorfoc_aEncoderAngle(ROLL));
		ST32DO_motorfoc_aEncoderAngle_Yaw,	// (s16)(100*motorfoc_aEncoderAngle(YAW));
		ST32DO_zero_0,						// (s16)(0.0f);
		ST32DO_zero_1,						// (s16)(0.0f); //link yaw
		ST32DO_Imu1AHRS_acc_mag,			// (s16)(10000*Imu1AHRS._acc_mag);
		ST32DO_Imu1AHRS_acc_confidence,		// (s16)(10000*Imu1AHRS._acc_confidence);
		ST32DO_functionInputPulsePacked,	// pack_functioninputvalues(&FunctionInputPulse);  //Function input values
		ST32DO_crc
	};

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

    bool getProcessing( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms, int& _speedSmoothRatio);
    bool setProcessing( int& _process_pulse_interval_ms, int& _process_speedsmooth_interval_ms, int _speedSmoothRatio);

    bool getMotorParams( int& _power, int& _scaleFactor, int& _expo );
    bool setMotorParams( int _power, int _scaleFactor, int _expo );

    bool getTimeouts( int& _mot_stop_nopulse_timeout_ms, int& _mot_disable_stopped_timeout_ms );
    bool setTimeouts( int _mot_stop_nopulse_timeout_ms, int _mot_disable_stopped_timeout_ms );

	bool getSensors( 
		double& _baroTimeStampSec, int& _baroTemperature, int& _baroPressure,
		double& _magnTimeStamp, int& _magnX, int& _magnY, int& _magnZ,
		double& _mpuTimeStamp, int& _accelX, int& _accelY, int& _accelZ, int& _gyroX, int& _gyroY, int& _gyroZ );

	bool getStorm32( std::vector< int>& _values );

	bool readStorm32LiveData( double& _timeStamp );

	bool getStorm32LiveData( int _offset, int& _data0, int& _data1, int& _data2 );

	bool getDebug( int _offset, int& _data );

	bool getYawConfig( 
		double& _pidP, double& _pidI, double& _pidD,
		int& _stabilizeSmoothFactor, int& _st32UpdateIntevalMs, int& _yawMaxSpeed );

	bool setYawConfig( 
		double _pidP, double _pidI, double _pidD,
		int _stabilizeSmoothFactor, int _st32UpdateIntevalMs, int _yawMaxSpeed );

private:

    bool checkConnected( const std::string& _signature );
    bool checkExpectedTokensCount( const std::vector< std::string >& _tokens, int _tokensCount, const std::string& _signature );
    bool checkStatus( const std::string& _status, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, int _v1, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, int _v1, int _v2, const std::string& _signature );
    bool checkStatus( const std::string& _status, int _v0, int _v1, int _v2, int _v3, const std::string& _signature );
    bool checkStatus( const std::string& _status, double _v0, double _v1, double _v2, int _v3, int _v4, int _v5, const std::string& _signature );

	static std::string buildMessage( char cmd )
	{
		std::string msg;
		msg += cmd;
		return msg;
	}

	template< typename T0 >
	static std::string buildMessage( char cmd, T0 _arg0 )
	{
		std::stringstream ss;
		ss << cmd << " " << _arg0 << "\n";
		return ss.str();
	}

	template< typename T0, typename T1 >
	static std::string buildMessage( char cmd, T0 _arg0, T1 _arg1 )
	{
		std::stringstream ss;
		ss << cmd << " " << _arg0 << " " << _arg1 << "\n";
		return ss.str();
	}

	template< typename T0, typename T1, typename T2 >
	static std::string buildMessage( char cmd, T0 _arg0, T1 _arg1, T2 _arg2 )
	{
		std::stringstream ss;
		ss << cmd << " " << _arg0 << " " << _arg1 << " " << _arg2 << "\n";
		return ss.str();
	}

	template< typename T0, typename T1, typename T2, typename T3 >
	static std::string buildMessage( char cmd, T0 _arg0, T1 _arg1, T2 _arg2, T3 _arg3 )
	{
		std::stringstream ss;
		ss << cmd << " " << _arg0 << " " << _arg1 << " " << _arg2 << " " << _arg3 << "\n";
		return ss.str();
	}

	template< typename T0, typename T1, typename T2, typename T3, typename T4 >
	static std::string buildMessage( char cmd, T0 _arg0, T1 _arg1, T2 _arg2, T3 _arg3, T4 _arg4 )
	{
		std::stringstream ss;
		ss << cmd << " " << _arg0 << " " << _arg1 << " " << _arg2 << " " << _arg3 << " " << _arg4 << "\n";
		return ss.str();
	}

	template< typename T0, typename T1, typename T2, typename T3, typename T4, typename T5 >
	static std::string buildMessage( char cmd, T0 _arg0, T1 _arg1, T2 _arg2, T3 _arg3, T4 _arg4, T5 _arg5 )
	{
		std::stringstream ss;
		ss << cmd << " " << _arg0 << " " << _arg1 << " " << _arg2 << " " << _arg3 << " " << _arg4 << " " << _arg5 << "\n";
		return ss.str();
	}

	bool validateDevice(std::string& _status);
	std::string sendReceive( const std::string& _msg);

	CSerial m_serial;
};

