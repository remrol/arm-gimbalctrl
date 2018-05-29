/**********************************************************************************************
 * Arduino PID Library - Version 1.2.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under the MIT License
 **********************************************************************************************/

#include "PID_v1.h"
#include <string.h>

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
void PID_PID(float* _pInput, float* _pOutput, float* _pSetpoint, float _Kp, float _Ki, float _Kd, int8_t _POn, int8_t _direction, uint32_t _timeStampMs, uint16_t _sampleTimeMs, struct PID* pid)
{
	memset(pid, 0, sizeof( struct PID ) );
	pid->m_pOutput = _pOutput;
    pid->m_pInput = _pInput;
    pid->m_pSetpoint = _pSetpoint;
    pid->m_inAuto = 0;
//	pid->m_outputSum = 0;	// TODO added

    PID_SetOutputLimits(-255, 255, pid);				//default output limit corresponds to the arduino pwm limits

    pid->m_sampleTimeMs = _sampleTimeMs;			//default Controller Sample Time is 0.1 seconds

    PID_SetControllerDirection(_direction, pid);
    PID_SetTunings2(_Kp, _Ki, _Kd, _POn, pid);

    pid->m_lastTimeMs = _timeStampMs - pid->m_sampleTimeMs;
}

/*Constructor (...)*********************************************************
 *    To allow backwards compatability for v1.1, or for people that just want
 *    to use Proportional on Error without explicitly saying so
 ***************************************************************************/

void PID_PID2(float* _pInput, float* _pOutput, float* _pSetpoint, float _Kp, float _Ki, float _Kd, int8_t _direction, uint32_t _timeStampMs, uint16_t _sampleTimeMs, struct PID* pid)
{
    PID_PID(_pInput, _pOutput, _pSetpoint, _Kp, _Ki, _Kd, PID_P_ON_E, _direction, _timeStampMs, _sampleTimeMs, pid);
}

int8_t PID_NeedCompute(struct PID* pid, uint32_t _timeStampMs)
{
	if(!pid->m_inAuto)
	return 0;

	uint32_t timeChange = (_timeStampMs - pid->m_lastTimeMs);

	if(timeChange>=pid->m_sampleTimeMs)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
int8_t PID_Compute( struct PID* pid, uint32_t _timeStampMs )
{
	if(!pid->m_inAuto) 
		return 0;

	uint32_t timeChange = (_timeStampMs - pid->m_lastTimeMs);

	if(timeChange>=pid->m_sampleTimeMs)
	{
		/*Compute all the working error variables*/
		float input = *pid->m_pInput;
		float error = *pid->m_pSetpoint - input;
		float dInput = (input - pid->m_lastInput);
		pid->m_outputSum += (pid->m_ki * error);

		/*Add Proportional on Measurement, if PID_P_ON_M is specified*/
		if(!pid->m_OnE) 
			pid->m_outputSum -= pid->m_kp * dInput;

		if(pid->m_outputSum > pid->m_outMax) 
			pid->m_outputSum = pid->m_outMax;
		else if(pid->m_outputSum < pid->m_outMin) 
			pid->m_outputSum = pid->m_outMin;

		/*Add Proportional on Error, if PID_P_ON_E is specified*/
		float output;
		if(pid->m_OnE) 
			output = pid->m_kp * error;
		else 
			output = 0;

		/*Compute Rest of PID Output*/
		output += pid->m_outputSum - pid->m_kd * dInput;

		if(output > pid->m_outMax) 
			output = pid->m_outMax;
		else if(output < pid->m_outMin) 
			output = pid->m_outMin;

		*pid->m_pOutput = output;

		/*Remember some variables for next time*/
		pid->m_lastInput = input;
		pid->m_lastTimeMs = _timeStampMs;
	    return 1;
   }
   else 
		return 0;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID_SetTunings2(float _Kp, float _Ki, float _Kd, int8_t _POn, struct PID* pid)
{
	if (_Kp<0 || _Ki<0 || _Kd<0) 
		return;

	pid->m_pOn = _POn;
	pid->m_OnE = _POn == PID_P_ON_E;

	pid->m_dispKp = _Kp; 
	pid->m_dispKi = _Ki; 
	pid->m_dispKd = _Kd;

	float sampleTimeSec = ((float)pid->m_sampleTimeMs)/1000;
	pid->m_kp = _Kp;
	pid->m_ki = _Ki * sampleTimeSec;
	pid->m_kd = _Kd / sampleTimeSec;

	if(pid->m_direction == PID_REVERSE )
	{
		pid->m_kp = -pid->m_kp;
		pid->m_ki = -pid->m_ki;
		pid->m_kd = -pid->m_kd;
	}
}

/* SetTunings(...)*************************************************************
 * Set Tunings using the last-rembered POn setting
 ******************************************************************************/
void PID_SetTunings(float _Kp, float _Ki, float _Kd, struct PID* pid)
{
    PID_SetTunings2(_Kp, _Ki, _Kd, pid->m_pOn, pid); 
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void PID_SetSampleTime(int _sampleTimeMs, struct PID* pid)
{
	if (_sampleTimeMs > 0)
	{
		float ratio  = (float)_sampleTimeMs / (float)pid->m_sampleTimeMs;
		pid->m_ki *= ratio;
		pid->m_kd /= ratio;
		pid->m_sampleTimeMs = (uint32_t)_sampleTimeMs;
	}
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID_SetOutputLimits(float _min, float _max, struct PID* pid)
{
	if(_min >= _max) 
		return;

	pid->m_outMin = _min;
	pid->m_outMax = _max;

	if(pid->m_inAuto)
	{
		if(*pid->m_pOutput > pid->m_outMax) 
			*pid->m_pOutput = pid->m_outMax;
		else if(*pid->m_pOutput < pid->m_outMin) 
			*pid->m_pOutput = pid->m_outMin;

		if(pid->m_outputSum > pid->m_outMax) 
			pid->m_outputSum = pid->m_outMax;
		else if(pid->m_outputSum < pid->m_outMin) 
			pid->m_outputSum = pid->m_outMin;
	}
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID_SetMode(int8_t _mode, struct PID* pid)
{
	int8_t newAuto = (_mode == PID_AUTOMATIC);
    if(newAuto && !pid->m_inAuto)
    {  /*we just went from manual to auto*/
		PID_Initialize(pid);
    }
    pid->m_inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID_Initialize( struct PID* pid)
{
	pid->m_outputSum = *pid->m_pOutput;
	pid->m_lastInput = *pid->m_pInput;
	if(pid->m_outputSum > pid->m_outMax) 
		pid->m_outputSum = pid->m_outMax;
	else if(pid->m_outputSum < pid->m_outMin) 
		pid->m_outputSum = pid->m_outMin;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a PID_DIRECT acting process (+Output leads
 * to +Input) or a PID_REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID_SetControllerDirection(int8_t _direction, struct PID* pid)
{
	if(pid->m_inAuto && _direction !=pid->m_direction)
	{
	    pid->m_kp = pid->m_kp;
		pid->m_ki = pid->m_ki;
		pid->m_kd = pid->m_kd;
	}
	pid->m_direction = _direction;
}

/* Status Funcions*************************************************************
 * Just because you set the _Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
float PID_GetKp( struct PID* pid)
{ 
	return pid->m_dispKp; 
}

float PID_GetKi(struct PID* pid)
{ 
	return pid->m_dispKi;
}

float PID_GetKd(struct PID* pid)
{ 
	return pid->m_dispKd;
}

int PID_GetMode(struct PID* pid)
{ 
	return pid->m_inAuto ? PID_AUTOMATIC : PID_MANUAL;
}

int PID_GetDirection(struct PID* pid)
{ 
	return pid->m_direction;
}

