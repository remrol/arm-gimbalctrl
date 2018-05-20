#ifndef STORM32_H
#define STORM32_H

#include <stdint.h>

struct Storm32LiveData
{
	uint16_t STATE;							// state
	uint16_t status;						// status
	uint16_t status2;						// status2
	uint16_t status3;						// status3
	int16_t	performanceMaxLoopDonex10;		// (s16)(Performance.MaxLoopdone*10); //performance
	uint16_t imuErrorsCount;				// imu_ntbus_geterrocnt() + imu_onboard_geterrocnt(); //errors
	uint16_t lipoVoltage;					// lipo_voltage(); //lipo_voltage;
	uint16_t loopTimeMillis;				// (u16)looptime.millis_32; //timestamp
	uint16_t cycleTime;						// (u16)(1.0E6f*FDT); //cycle time
	int16_t aImu1AnglePitch;				// (s16)(100*aImu1Angle.Pitch); //Imu1 angles, in 0.01°
	int16_t aImu1AngleRoll;					// (s16)(100*aImu1Angle.Roll);
	int16_t aImu1AngleYaw;					// (s16)(100*aImu1Angle.Yaw);
	int16_t Imu1AHRS_R_x;					// (s16)(10000*Imu1AHRS.R.x); //Imu1 R estimates, in 0.0001 g
	int16_t Imu1AHRS_R_y;					// (s16)(10000*Imu1AHRS.R.y);
	int16_t Imu1AHRS_R_z;					// (s16)(10000*Imu1AHRS.R.z);
	int16_t aImu1AnglePitch_minus_PID;		// (s16)(100*cPID[PITCH].Cntrl - aImu1Angle.Pitch); //relative PID output, in 0.01°
	int16_t aImu1AngleRoll_minus_PID;		// (s16)(100*cPID[ROLL].Cntrl - aImu1Angle.Roll);
	int16_t aImu1AngleYaw_minus_PID;		// (s16)(100*cPID[YAW].Cntrl - aImu1Angle.Yaw);
	int16_t InputSrcPitch;					// InputSrc.Pitch; //Rc Input values
	int16_t	InputSrcRoll;					// InputSrc.Roll;
	int16_t	InputSrcYaw;					// InputSrc.Yaw;
	int16_t aImu2AnglePitch;				// (s16)(100*aImu2Angle.Pitch); //Imu2 angles, in 0.01°
	int16_t aImu2AngleRoll;					// (s16)(100*aImu2Angle.Roll);
	int16_t aImu2AngleYaw;					// (s16)(100*aImu2Angle.Yaw);
	int16_t motorfoc_aEncoderAnglePitch;	// (s16)(100*motorfoc_aEncoderAngle(PITCH)); //Encoder angles, in 0.01°
	int16_t motorfoc_aEncoderAngle_Roll;	// (s16)(100*motorfoc_aEncoderAngle(ROLL));
	int16_t motorfoc_aEncoderAngle_Yaw;		// (s16)(100*motorfoc_aEncoderAngle(YAW));
	int16_t zero_0;							// (s16)(0.0f);
	int16_t zero_1;							// (s16)(0.0f); //link yaw
	int16_t Imu1AHRS_acc_mag;				// (s16)(10000*Imu1AHRS._acc_mag);
	int16_t Imu1AHRS_acc_confidence;		// (s16)(10000*Imu1AHRS._acc_confidence);
	uint16_t functionInputPulsePacked;		// pack_functioninputvalues(&FunctionInputPulse);  //Function input values
	uint16_t crc;
	uint8_t endChar;						// o
};

#define STORM32_FIELDSCOUNT 32

typedef uint8_t Storm32Status;
static const Storm32Status ST32_UPDATE_OK = 0;
static const Storm32Status ST32_UPDATE_TIMEOUT = 1;
static const Storm32Status ST32_UPDATE_UARTERROR = 2;


Storm32Status storm32_UpdateStatus();


extern uint32_t        g_storm32LiveDataTimeStamp;
extern Storm32LiveData g_storm32LiveData;


#endif // STORM32_H


