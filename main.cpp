#define __STDC_LIMIT_MACROS
#include "fcpu.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "i2c_master.h"
#include "calculations.h"
#include "config.h"
#include "control.h"
#include "sensors.h"
#include "storm32.h"
#include "debug.h"

extern "C"
{
#include "uart.h"
};

#include "time.h"


//--------------------------------------------------------------------------------
void setMotorSpeed( int16_t speed );	
void handleSpeedSmooth();
void calcYawCtrSpeed( uint16_t pulseMs );


ISR(TIMER1_OVF_vect)
{	
	// Clear overflow flag.
	TIFR &= 0xff ^ _BV(TOV1);				

	// Increment overflows count.
	g_state.vT1OverflowCount += 0x10000;					
}	

ISR(TIMER3_OVF_vect)
{
	// Clear overflow flag.
	ETIFR &= 0xff ^ _BV(TOV3);

	// Increment overflows count.
	g_state.vT3OverflowCount += 0x10000;
}

ISR(TIMER1_CAPT_vect)
{
	static uint32_t risingEdgeTimeStamp = 0;// Rising edge time stamp, 0 means non-valid.
	
	// Critical operations.
	uint8_t risingEdge = TCCR1B & _BV( ICES1 );		// Read which edge has been captured.
	TCCR1B ^= _BV( ICES1 );					// Switch edge capture.
	
	uint32_t timeStamp = g_state.vT1OverflowCount;	// Compute precise 32 bit time stamp.
	timeStamp |= ICR1;
	
	if( risingEdge ) // Read which edge has been captured.
	{
		// If rising edge then remember its time stamp.
		risingEdgeTimeStamp = timeStamp;
	}
	else if( risingEdgeTimeStamp != 0 )
	{
		// Compute pulse duration.
		uint32_t pulseDuration = ( timeStamp - risingEdgeTimeStamp ) / ( F_CPU / 8000000 );
		// Clear rising edge.
		risingEdgeTimeStamp = 0;
	
		// Validate measured pulse duration. Accept only 500 - 2500 us range.	
		if( pulseDuration >= 500 && pulseDuration <= 2500 )
		{
			g_state.vPulse1DurationSum += pulseDuration;
			g_state.vPulse1DurationSumCount += 1;
			g_state.vPulse1TimeStamp = millis();		// Store pulse time stamp.
		}
	}
	else
	{
		// Should not happen, we have falling edge but no rising edge time stamp captured.
	}
}

ISR(TIMER3_CAPT_vect)
{
	static uint32_t risingEdgeTimeStamp = 0;// Rising edge time stamp, 0 means non-valid.
	
	// Critical operations.
	uint8_t risingEdge = TCCR3B & _BV( ICES3 );		// Read which edge has been captured.
	TCCR3B ^= _BV( ICES3 );					// Switch edge capture.
	
	uint32_t timeStamp = g_state.vT3OverflowCount;	// Compute precise 32 bit time stamp.
	timeStamp |= ICR3;
	
	if( risingEdge ) // Read which edge has been captured.
	{
		// If rising edge then remember its time stamp.
		risingEdgeTimeStamp = timeStamp;
	}
	else if( risingEdgeTimeStamp != 0 )
	{
		// Compute pulse duration.
		uint32_t pulseDuration = ( timeStamp - risingEdgeTimeStamp ) / ( F_CPU / 8000000 );
		// Clear rising edge.
		risingEdgeTimeStamp = 0;
		
		// Validate measured pulse duration. Accept only 500 - 2500 us range.
		if( pulseDuration >= 500 && pulseDuration <= 2500 )
		{
			g_state.vPulse3DurationSum += pulseDuration;
			g_state.vPulse3DurationSumCount += 1;
			g_state.vPulse3TimeStamp = millis();		// Store pulse time stamp.
		}
	}
	else
	{
		// Should not happen, we have falling edge but no rising edge time stamp captured.
	}
}

ISR(TIMER0_COMP_vect)
{
	// React only on rising edge
//	if( PORTB & _BV( 3 ) )
	{
		g_state.vMotorPosition += g_state.motorDirection;
	}
}

//-------------------------------------------------------------------------------------

void calcYawCtrSpeed( uint16_t pulseMs )
{
	static uint16_t lastPulseMs = 0;
	uint16_t diff;
	
	// Skip processing if pulse haven't changed
	if( pulseMs == lastPulseMs )
		return;
	
	lastPulseMs = pulseMs;
	
	if( pulseMs == 0 )
	{
		// Stop motor if out of range.
		g_state.yawCtrlSpeed = 0;
	}
	else
	{
		if( pulseMs >= g_config.pulse_dband_hi )
		{
			// Positive rotation.
			if( pulseMs > g_config.pulse_max )
				pulseMs = g_config.pulse_max;
	
			//	diff = ( pulseMs - g_config.pulse_dband_hi * g_config.power;  diff /= ( g_config.pulse_max - g_config.pulse_dband_hi );
			diff = exponent(pulseMs - g_config.pulse_dband_hi, g_config.pulse_max - g_config.pulse_dband_hi, g_config.expo_percent );
			diff = diff * g_config.power / ( g_config.pulse_max - g_config.pulse_dband_hi );
			g_state.yawCtrlSpeed = diff;
		}
		else if( pulseMs <= g_config.pulse_dband_lo )
		{
			// Negative rotation.
			if( pulseMs < g_config.pulse_min)
				pulseMs = g_config.pulse_min;

			//			diff = ( g_config.pulse_dband_lo - pulseMs ) * g_config.power;	diff /= g_config.pulse_dband_lo - g_config.pulse_min;
			diff = exponent( g_config.pulse_dband_lo - pulseMs, g_config.pulse_dband_lo - g_config.pulse_min, g_config.expo_percent );
			diff = diff * g_config.power / ( g_config.pulse_dband_lo - g_config.pulse_min );
			g_state.yawCtrlSpeed = -diff;
		}
		else
		{
			// In dead band, stop the motor.
			g_state.yawCtrlSpeed = 0;
		}
	}
}

//-------------------------------------------------------------------------------------

void setMotorSpeed( int16_t speed )
{
	// cpu clock / 64
	#define CLOCK0_SELECT ( _BV( CS02 ) | _BV( CS01 ) | _BV( CS00 ))
	
	// Time stamp of last motor move action.
	static uint32_t moveTimeStamp = 0;
	
	// Diagnostics, get speed min/max

	// Special handling for zero speed
	if( speed == 0 )
	{
		// Stopping motor for the very first time
		if( g_state.motorSpeed != 0 )
		{
			g_state.motorDirection = 0;
			g_state.motorSpeed = 0;
			TCCR0 &= 0xff ^ ( _BV( CS02 ) | _BV( CS01 ) | _BV( CS00 ) );	// Stop clock
		}
		
		// Shutdown motor after 10 seconds to minimize power consumption
		if( millis() > moveTimeStamp + g_config.mot_disable_stopped_timeout_ms )
		{
			PORTB |=  _BV(PB1); //0x02;			// set PB1 (NENABLE)
		}
		
		return;
	}
	
	// Remember motor move time stamp as we are moving now
	moveTimeStamp = millis();
	
	// Check if speed really differs
	if( speed == g_state.motorSpeed )
		return;
	
	else if( speed > 0 )
	{
		int16_t ocr0 = ( g_config.pwm_scale_factor / speed ) - 1;
		g_state.ocr0 = ocr0 < 255 ? ocr0 : 255;
		OCR0 = g_state.ocr0;
		g_state.motorDirection = 1;
		
		if( g_state.motorSpeed <= 0 ) // Handle speed direction change
		{
			TCCR0 |= CLOCK0_SELECT;	// Set clock
			uint8_t portbState = PORTB;
			portbState |= _BV(PB2);			// Set PB2 (dir)
			portbState &= 0xff ^ _BV(PB1);	// Clear PB1 (NENABLE)
			PORTB = portbState;
		}
	}
	else
	{
		int16_t ocr0 = ( g_config.pwm_scale_factor / -speed ) - 1;
		g_state.ocr0 = ocr0 < 255 ? ocr0 : 255;
		OCR0 = g_state.ocr0;
		g_state.motorDirection = -1;
		
		if( g_state.motorSpeed >= 0 )	// Handle direction change
		{
			TCCR0 |= CLOCK0_SELECT;
			uint8_t portbState = PORTB;
			portbState &= 0xff ^ _BV(PB2);	// Clear PB2 (dir)
			portbState &= 0xff ^ _BV(PB1);	// Clear PB1 (NENABLE)
			PORTB = portbState;
		}
	}
	
	g_state.motorSpeed = speed;
}

void handleSpeedSmooth()
{
	int16_t dstSpd;
	
	// Rewrite to get rid of overwrites
	int16_t speed = g_state.yawCtrlSpeed;
	int8_t speedSmoothFactor = g_config.speed_normal_smooth_factor;
	
	// if yaw stabilize then assign speed from yaw error and increase smooth factor
	if( g_state.yawStabilizeMode)
	{
		speed =  g_state.yawPIDspeed;
		if( speed < -g_config.yawMaxSpeed )
			speed = -g_config.yawMaxSpeed;
		else if( speed > g_config.yawMaxSpeed )
			speed = g_config.yawMaxSpeed;
			
		speedSmoothFactor = g_config.yaw_speed_smooth_factor;
	}
	
	if( speed == g_state.motorSpeed )
	{
		setMotorSpeed( g_state.motorSpeed );	
	}
	else if( speed < g_state.motorSpeed )
	{
		dstSpd = g_state.motorSpeed - speedSmoothFactor;
		setMotorSpeed( dstSpd > speed ? dstSpd : speed );
	}
	else
	{
		dstSpd = g_state.motorSpeed + speedSmoothFactor;
		setMotorSpeed( dstSpd < speed ? dstSpd : speed );	
	}
}


uint16_t getPulse1Time()
{
	uint32_t pulseDurationSum;
	uint16_t pulseDurationSumCount;

	// Rewrite volatile vars with interrupts off.
	uint8_t oldSREG = SREG;
	cli();
	pulseDurationSum = g_state.vPulse1DurationSum;
	pulseDurationSumCount = g_state.vPulse1DurationSumCount;
	g_state.vPulse1DurationSum = 0;
	g_state.vPulse1DurationSumCount = 0;
	SREG = oldSREG;

	// Calc avg pulse time
	if( pulseDurationSumCount == 0 )
		return 0;

	return pulseDurationSum / pulseDurationSumCount;
}

uint32_t getPulse1TimeStamp()
{
	uint32_t pulseTimeStamp;
	uint8_t oldSREG = SREG;
	cli();
	pulseTimeStamp = g_state.vPulse1TimeStamp;
	SREG = oldSREG;

	return pulseTimeStamp;
}

uint16_t getPulse3Time()
{
	uint32_t pulseDurationSum;
	uint16_t pulseDurationSumCount;

	// Rewrite volatile vars with interrupts off.
	uint8_t oldSREG = SREG;
	cli();
	pulseDurationSum = g_state.vPulse3DurationSum;
	pulseDurationSumCount = g_state.vPulse3DurationSumCount;
	g_state.vPulse3DurationSum = 0;
	g_state.vPulse3DurationSumCount = 0;
	SREG = oldSREG;

	// Calc avg pulse time
	if( pulseDurationSumCount == 0 )
		return 0;

	return pulseDurationSum / pulseDurationSumCount;
}
	

int16_t subtractAngles( int16_t _angle1, int16_t _angle2 )
{
	if( _angle1 < -9000 && _angle2 > 9000 )
	{
		return ( (int16_t)18000 + _angle1 ) + ( (int16_t)18000 - _angle2 );
	}
	else if( _angle1 > 9000 && _angle2 < -9000 )
	{
		return -( ( (int16_t)18000 - _angle1 ) + ( (int16_t)18000 + _angle2 ) );
	}
	else
	{
		return _angle1 - _angle2;
	}
}
	
int16_t addAngles( int16_t _angle1, int16_t _angle2 )
{
	int16_t angle = _angle1 + _angle2;
	
	// wrap-around at -18000 and  18000 (-180 deg, 180 deg)
	if( angle < -18000 )
	{
		angle = ( angle + 18000 ) + 18000;
	}
	else if( angle > 18000 )
	{
		angle = ( angle - 18000 ) - 18000;
	}
	
	return angle;
}
	
bool yawAvailable()
{
	// Timestamp is valid and not older than 1 sec
	return g_state.storm32YawTimeStamp != 0 && g_state.storm32YawTimeStamp + 1000 > millis();
}

void handleYawStabilizeMode()
{
	if( g_state.pulse3Duration > 1500 && g_state.pulse3Duration < 2300 && yawAvailable() )
	{
		// If entering stabilize mode then read current position as reference position
		if( g_state.yawStabilizeMode == 0 )
		{
			g_state.yawStabilizeMode = 1;
			g_state.yawOffset = g_state.storm32YawAngle;
			g_state.yawPIDspeed = 0;
			
			// PID.
			g_state.yawPIDInput = g_state.storm32YawAngle;
			g_state.yawPIDSetPoint = g_state.storm32YawAngle;
			
			PID_PID2( &g_state.yawPIDInput,  &g_state.yawPIDOutput, &g_state.yawPIDSetPoint,
				g_config.yawPID_p, g_config.yawPID_i, g_config.yawPID_d, PID_DIRECT, millis(), 10, &g_state.yawPID );
			PID_SetOutputLimits( -g_config.yawMaxSpeed, g_config.yawMaxSpeed, &g_state.yawPID );
			PID_SetMode( PID_AUTOMATIC, &g_state.yawPID );
		}
	}
	else
	{
		g_state.yawStabilizeMode = 0;
	}
}
	
int main(void)
{
	debug_init();
	configEepromLoad();
	stateInit();
	
	// OUTPUT --------------------------------------------------------------------
	// WAVE out OC0 = PB4 (STEP)
	//                PB2 (DIR)
	//                PB1 (NENABL)
	//
	// freq = fclk / ( 2 * N * (1 + OCR1 ) ) 
	// fmin = Fclk / ( 2 * N * 256 )
	// fmax = fclk / ( 2 * N )
	
	DDRB = 0xff;		// Whole B as output	
	
	DDRA = 0xff;	// PA out

	TCCR0 = _BV( WGM01 ) |								// CTC
			/*_BV( CS02 ) | _BV( CS01 ) | _BV( CS00 ) | // clock prescaler */
			_BV( COM00 );								// Set OC0 on compare match when up-counting. Clear OC0 on compare match when down counting
	
	// Input -----------------------------------------------------------------
	// Capture source ICP1 = PD4
	// PD2 output, rest input
	DDRD = _BV(2);
	TCCR1B = _BV( ICNC1 ) |	// Allow noise canceler
			 _BV( CS11 )  |	// Clock / 8
			 _BV( ICES1 );	// A rising edge is capture event.
			 
			 
	// Capture source ICP3 = PE7
	DDRE = 0x01; // PE0 = RxD = input, PE1 = TxD = output
	
	TCCR3B = _BV(ICNC3 ) |	// Allow noise canceler
			 _BV(CS31)	 |  // Clock/8
			 _BV(ICES3);	// A rising edge is capture event.

	// Timer millis()
	initTime();
			
	// Interrupts -------------------------------------------------------------
	TIMSK = _BV( TOIE2 ) |  // Enable overflow interrupt, for timer millis()
	    	_BV( TOIE1 )  |	// Enable interrupt by timer1 overflow
			_BV( TICIE1 ) | // Enable interrupt by timer1 capture event
			_BV( OCIE0 );  	// Enable interrupt by timer0 output compare match
	ETIMSK = _BV(TOIE3) |	// Enable interrupt by timer3 overflow
			 _BV(TICIE3);	// Enable interrupt by timer3 capture event.
	
	TIFR = 0;				// Clear flags
	
	// I2C and sensors --------------------------------------------------------
	sensorsInit();
	storm32Init();

	// Main loop --------------------------------------------------------------
	sei();
	
	// Main uart
	uart_init(UART_BAUD_SELECT(19200, F_CPU));
	// storm32 uart
	uart1_init(UART_BAUD_SELECT(38400, F_CPU));

	//---------------------
	
	uint32_t handleSpeedSmoothTimeout, handlePulseTimeout, handleStorm32UpdateTimeout;
	handleStorm32UpdateTimeout = handleSpeedSmoothTimeout = handlePulseTimeout = millis();
	handleStorm32UpdateTimeout += 5*1000; // Delay storm32 update 5 seconds
	
    while(1)
    {
		uint32_t now = millis();

		// Periodically pulse duration -> speed handler
		if( now >= handlePulseTimeout )
		{
			g_state.pulse1Duration = getPulse1Time();
			g_state.pulse3Duration = getPulse3Time();
					
			//  Update yaw ctrl speed if pulse value available or timeout exceeded
			if( g_state.pulse1Duration != 0 || ( now > getPulse1TimeStamp() + g_config.mot_stop_nopulse_timeout_ms) )
			{
				calcYawCtrSpeed(g_state.pulse1Duration);
			}
			
			handleYawStabilizeMode();
								
			handlePulseTimeout += g_config.process_pulse_interval_ms;
			now = millis();
		}

		// Periodically handle speed smoother 
		if( now >= handleSpeedSmoothTimeout )
		{
			handleSpeedSmooth();
			handleSpeedSmoothTimeout += g_config.process_speedsmooth_interval_ms;
			now = millis();
		}

		// Periodically update storm32 data
		if( now >= handleStorm32UpdateTimeout )
		{
			// Get fresh data from storm32
			storm32UpdateAngles();

			// PID
			if( g_state.yawStabilizeMode )
			{
				// Update yaw offset by rotation speed
				g_state.yawOffset = addAngles( g_state.yawOffset, g_state.yawCtrlSpeed / 2 );
				// PID input
				g_state.yawPIDInput = g_state.storm32YawAngle;
				g_state.yawPIDSetPoint = g_state.yawOffset;
				// PID execute
				PID_Compute( &g_state.yawPID, millis());
				// PID output
				g_state.yawPIDspeed = g_state.yawPIDOutput;
			
				// Debug
				g_debug.data0 = g_state.yawPIDOutput;
				g_debug.data1 = subtractAngles( g_state.yawOffset, g_state.storm32YawAngle );
				g_debug.data2 = g_state.storm32YawAngle;
			}
			
			handleStorm32UpdateTimeout += g_config.storm32_update_inteval_ms;
		}
		
		// Read sensors
		sensorsRead();

		// IO control
		control();
    }
}




