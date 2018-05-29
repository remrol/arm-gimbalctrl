#ifndef PID_v1_h
#define PID_v1_h

// LIBRARY_VERSION	1.2.1

#include <stdio.h>

//Constants used in some of the functions below
#define PID_AUTOMATIC	1
#define PID_MANUAL		0

#define PID_DIRECT  0
#define PID_REVERSE  1

#define PID_P_ON_M 0
#define PID_P_ON_E 1


struct PID
{
	float m_dispKp;				// * we'll hold on to the tuning parameters in user-entered 
	float m_dispKi;				//   format for display purposes
	float m_dispKd;				//
    
	float m_kp;                  // * (P)roportional Tuning Parameter
    float m_ki;                  // * (I)ntegral Tuning Parameter
    float m_kd;                  // * (D)erivative Tuning Parameter

	int8_t m_direction;
	int8_t m_pOn;

    float *m_pInput;              // * Pointers to the Input, Output, and Setpoint variables
    float *m_pOutput;             //   This creates a hard link between the variables and the 
    float *m_pSetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
			  
	uint32_t m_lastTimeMs;
	float m_outputSum;
	float m_lastInput;

	uint32_t m_sampleTimeMs;
	float m_outMin;
	float m_outMax;
	int8_t m_inAuto;
	int8_t m_OnE;
};


//commonly used functions **************************************************************************
// * constructor.  links the PID to the Input, Output, and
//   Setpoint.  Initial tuning parameters are also set here.
//   (overload for specifying proportional mode)
void PID_PID(float* _pInput, float* _pOutput, float* _pSetpoint, float _Kp, float _Ki, float _Kd, int8_t _POn, int8_t _direction, uint32_t _timeStampMs, uint16_t _sampleTimeMs, struct PID* pid);

// * constructor.  links the PID to the Input, Output, and
//   Setpoint.  Initial tuning parameters are also set here
void PID_PID2(float* _pInput, float* _pOutput, float* _pSetpoint, float _Kp, float _Ki, float _Kd, int8_t _direction, uint32_t _timeStampMs, uint16_t _sampleTimeMs, struct PID* pid);
  
// * sets PID to either Manual (0) or Auto (non-0)
void PID_SetMode(int8_t _mode, struct PID* pid);               

int8_t PID_NeedCompute(struct PID* pid, uint32_t _timeStampMs);

// * performs the PID calculation.  it should be
//   called every time loop() cycles. ON/OFF and
//   calculation frequency can be set using SetMode
//   SetSampleTime respectively
int8_t PID_Compute(struct PID* pid, uint32_t _timeStampMs);


// * clamps the output to a specific range. 0-255 by default, but
//   it's likely the user will want to change this depending on
//   the application
void PID_SetOutputLimits(float _min, float _max, struct PID* pid);

//available but not commonly used functions ********************************************************

// * While most users will set the tunings once in the
//   constructor, this function gives the user the option
//   of changing tunings during runtime for Adaptive control
void PID_SetTunings(float _Kp, float _Ki, float _Kd, struct PID* pid);

// * overload for specifying proportional mode
void PID_SetTunings2(float _Kp, float _Ki, float _Kd, int8_t _POn, struct PID* pid);

// * Sets the Direction, or "Action" of the controller. PID_DIRECT
//   means the output will increase when error is positive. PID_REVERSE
//   means the opposite.  it's very unlikely that this will be needed
//   once it is set in the constructor.
void PID_SetControllerDirection(int8_t _direction, struct PID* pid);

// * sets the frequency, in Milliseconds, with which
//   the PID calculation is performed.  default is 100
void PID_SetSampleTime(int _sampleTimeMs, struct PID* pid);
  
//Display functions ****************************************************************
float PID_GetKp(struct PID* pid);						  // These functions query the pid for interal values.
float PID_GetKi(struct PID* pid);						  //  they were created mainly for the pid front-end,
float PID_GetKd(struct PID* pid);						  // where it's important to know what is actually
int PID_GetMode(struct PID* pid);						  //  inside the PID.
int PID_GetDirection(struct PID* pid);					  //
void PID_Initialize( struct PID* pid);

#endif

