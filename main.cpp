#include "fcpu.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>

//#include "simple_uart.h"
#include "i2c_master.h"
#include "calculations.h"
#include "config.h"
#include "control.h"

extern "C"
{
#include "uart.h"
#include "time.h"
};


//--------------------------------------------------------------------------------
void setMotorSpeed( int16_t speed );	
void handleSpeedSmooth();
void pulseDurationToSpeed( uint16_t pulseMs );


ISR(TIMER1_OVF_vect)
{	
	// Clear overflow flag.
	TIFR &= 0xff ^ _BV(TOV1);				

	// Increment overflows count.
	g_state.vT1OverflowCount += 0x10000;					
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
			g_state.vPulseDurationSum += pulseDuration;
			g_state.vPulseDurationSumCount += 1;
			g_state.vPulseTimeStamp = millis();		// Store pulse time stamp.
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

void pulseDurationToSpeed( uint16_t pulseMs )
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
		g_state.speed = 0;
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
			g_state.speed = diff;
		}
		else if( pulseMs <= g_config.pulse_dband_lo )
		{
			// Negative rotation.
			if( pulseMs < g_config.pulse_min)
				pulseMs = g_config.pulse_min;

			//			diff = ( g_config.pulse_dband_lo - pulseMs ) * g_config.power;	diff /= g_config.pulse_dband_lo - g_config.pulse_min;
			diff = exponent( g_config.pulse_dband_lo - pulseMs, g_config.pulse_dband_lo - g_config.pulse_min, g_config.expo_percent );
			diff = diff * g_config.power / ( g_config.pulse_dband_lo - g_config.pulse_min );
			g_state.speed = -diff;
		}
		else
		{
			// In dead band, stop the motor.
			g_state.speed = 0;
		}
	}
}

//-------------------------------------------------------------------------------------

void setMotorSpeed( int16_t speed )
{
	// cpu clock / 64
	#define CLOCK0_SELECT ( _BV( CS02 ) | _BV( CS01 ) | _BV( CS00 ))
	
	uint8_t  portbState;
	
	// Time stamp of last motor move action.
	static uint32_t moveTimeStamp = 0;
	
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
		if( millis() > moveTimeStamp + MOT_DISABLE_TIMEOUT_MS )
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
			portbState = PORTB;
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
			portbState = PORTB;
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
	int16_t speed = g_state.speed;
	
	if( speed == g_state.motorSpeed )
	{
		setMotorSpeed( g_state.motorSpeed );	
	}
	else if( speed < g_state.motorSpeed )
	{
		dstSpd = g_state.motorSpeed - g_config.speed_smooth_factor;
		setMotorSpeed( dstSpd > speed ? dstSpd : speed );
	}
	else
	{
		dstSpd = g_state.motorSpeed + g_config.speed_smooth_factor;
		setMotorSpeed( dstSpd < speed ? dstSpd : speed );	
	}
}


uint16_t getPulseTime()
{
	uint32_t pulseDurationSum;
	uint16_t pulseDurationSumCount;

	// Rewrite volatile vars with interrupts off.
	uint8_t oldSREG = SREG;
	cli();
	pulseDurationSum = g_state.vPulseDurationSum;
	pulseDurationSumCount = g_state.vPulseDurationSumCount;
	g_state.vPulseDurationSum = 0;
	g_state.vPulseDurationSumCount = 0;
	SREG = oldSREG;

	// Calc avg pulse time
	if( pulseDurationSumCount == 0 )
		return 0;

	return pulseDurationSum / pulseDurationSumCount;
}

uint32_t getPulseTimeStamp()
{
	uint32_t pulseTimeStamp;
	uint8_t oldSREG = SREG;
	cli();
	pulseTimeStamp = g_state.vPulseTimeStamp;
	SREG = oldSREG;

	return pulseTimeStamp;
}


int main(void)
{
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
	// Capture source = ICP1 = PD4	
	// Whole D as input
	DDRD = 0x00;
	
	TCCR1B = _BV( ICNC1 ) |	// Allow noise canceler
			 _BV( CS11 )  |	// Clock / 8
			 _BV( ICES1 );	// A rising edge is capture event.
			 

	// Timer millis()
	initTime();
			
	// Interrupts -------------------------------------------------------------
	TIMSK = _BV( TOIE2 ) |  // Enable overflow interrupt, for timer millis()
	    	_BV( TOIE1 )  |	// Enable interrupt by timer1 overflow
			_BV( TICIE1 ) | // Enable interrupt by timer1 capture event
			_BV( OCIE0 );  	// Enable interrupt by timer0 output compare match
	
	TIFR = 0;				// Clear flags
	
	// I2C --------------------------------------------------------------------
	i2c_init();
	
	// Main loop --------------------------------------------------------------
	sei();
	
	DDRE = 0x01; // PE0 = RxD = input, PE1 = TxD = output

	uart_init(UART_BAUD_SELECT(19200, F_CPU));
	//---------------------
	
//	HMC5883L_init();
	
	uint32_t handleSpeedSmoothTimeout, handlePulseTimeout;
	handleSpeedSmoothTimeout = handlePulseTimeout = millis();
	

    while(1)
    {
		uint32_t now = millis();

		// Periodically pulse duration -> speed handler
		if( now >= handlePulseTimeout )
		{
			g_state.pulseDuration = getPulseTime();
			if( g_state.pulseDuration > 0 )
			{
				pulseDurationToSpeed(g_state.pulseDuration);
			}
			else if( now > getPulseTimeStamp() + MOT_STOP_NO_PULSE_TIMEOUT_MS)
			{
				//  If no response for 2 seconds then set pulse duration to invalid.
				pulseDurationToSpeed(g_state.pulseDuration);
			}
			
			handlePulseTimeout += g_config.process_pulse_interval_ms;
		}

		// Periodically handle speed smoother 
		if( now >= handleSpeedSmoothTimeout )
		{
			handleSpeedSmooth();
			handleSpeedSmoothTimeout += g_config.process_speedsmooth_interval_ms;
		}

		// IO control
		control();
    }
}




