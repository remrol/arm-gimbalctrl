#define F_CPU 8000000


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>

#include "simple_uart.h"
#include "i2c_master.h"

// Pulse duration measure, Working (private) variables --------------------------
// Timestamp of last measured pulse
volatile uint16_t g_lastPulseTime = 0;

volatile uint32_t risingEdgeTimeStamp = 0;		// Rising edge time stamp, 0 means non-valid.

#define PULSE_DURATION_SCALE 2
// Public value: last measured pulse duration in microseconds, 0 if not available.
volatile uint16_t g_pulseDuration = 0;	// Last measured pulse duration.

// Direction
volatile int8_t g_actualDirection = 0;
volatile int8_t g_actualSpeed = 0;
volatile int8_t g_speed = 0;

// Motor position
volatile uint16_t g_motorPosition = 0;

// time since start in seconds
volatile uint16_t g_timeSec = 0;

void HMC5883L_init(void);
void HMC5883L_read(void);


//-------------------------------------------------------------------------------
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);

int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
	usart_send('\r');
	
	usart_send( c );
	
	return 0;
}

int uart_getchar(FILE *stream)
{
	return usart_receive();
}

static FILE mystdout;
	
//--------------------------------------------------------------------------------
void motorSpeed( int8_t speed );	
void handleSpeed(void);
void processPulse( uint16_t pulseMs );

volatile uint32_t timer1OverflowCount = 0;				// Timer 1 overflow count.

ISR(TIMER1_OVF_vect)
{	
	TIFR &= 0xff ^ _BV(TOV1);				// Clear overflow flag.

	timer1OverflowCount += 1;					// Increment overflows count.

	//  If no response for 2 seconds then set pulse duration to invalid.
	if( g_timeSec - g_lastPulseTime >= 2 )
	{
		g_pulseDuration = 0;
		processPulse( g_pulseDuration );
	}
}	

ISR(TIMER1_CAPT_vect)
{
	uint8_t risingEdge;
	uint32_t timeStamp;
	uint32_t pulseDuration;
	
	// Critical operations.
	risingEdge = TCCR1B & _BV( ICES1 );		// Read which edge has been captured.
	TCCR1B ^= _BV( ICES1 );					// Switch edge capture.
	
	timeStamp = timer1OverflowCount << 16;	// Compute precise 32 bit time stamp.
	timeStamp |= ICR1;
	
	if( risingEdge ) // Read which edge has been captured.
	{
		// If rising edge then remember its time stamp.
		risingEdgeTimeStamp = timeStamp;
	}
	else if( risingEdgeTimeStamp != 0 )
	{
		// Compute pulse duration.
		pulseDuration = ( timeStamp - risingEdgeTimeStamp ) / ( F_CPU / 4000000 );
		// Clear rising edge.
		risingEdgeTimeStamp = 0;
	
		// Validate measured pulse duration. Accept only 500 - 2500 us range.	
		if( pulseDuration >= 700 / PULSE_DURATION_SCALE && pulseDuration <= 2300 / PULSE_DURATION_SCALE )
		{
			g_pulseDuration = pulseDuration;	// Store pulse duration.
			g_lastPulseTime = g_timeSec;		// Store pulse time stamp.
			
			processPulse( g_pulseDuration );
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
	if( PORTB & _BV( 3 ) )
		g_motorPosition += g_actualDirection;
}

//-------------------------------------------------------------------------------------

// Timer2 interrupt, approx 200 times/second
ISR(TIMER2_COMP_vect)
{
	static uint8_t slope = 0;
	
	// Update time stamp in seconds
	if( slope == 200 )
	{
		slope = 0;
		g_timeSec++;
	}
	else
	{
		++slope;
	}
	
	// Update motor speed
	handleSpeed();	
}

void handleSpeed()
{
	int8_t dstSpd;
	// Rewrite to get rid of overwrites
	int8_t spd = g_speed;
	
	if( spd == g_actualSpeed )
	{
		motorSpeed( g_actualSpeed );	
	}
	else if( spd < g_actualSpeed )
	{
		dstSpd = g_actualSpeed - 1;
		motorSpeed( dstSpd > spd ? dstSpd : spd );
	}
	else
	{
		dstSpd = g_actualSpeed + 1;
		motorSpeed( dstSpd < spd ? dstSpd : spd );	
	}
}

void motorSpeed( int8_t speed )
{
	#define FREF 3000
	// cpu clock / 64
	#define CLOCK0_SELECT ( _BV( CS02 ) | _BV( CS00 ) )
	
	uint8_t  portbState;
	
	// Time stamp of last motor move action.
	static uint16_t moveTimeStamp = 0;
			
	// Special handling for zero speed
	if( speed == 0 )
	{
		// Stopping motor for the very first time
		if( g_actualSpeed != 0 )
		{
			g_actualDirection = 0;
			g_actualSpeed = 0;
			TCCR0 &= 0xff ^ ( _BV( CS02 ) | _BV( CS01 ) | _BV( CS00 ) );	// Stop clock
		}
		
		// Shutdown motor after 10 seconds to minimize power consumption
		if( g_timeSec - moveTimeStamp >= 10 )
		{
			PORTB |=  _BV(PB1); //0x02;			// set PB1 (NENABLE)
		}
		
		return;
	}
	
	// Remember motor move time stamp as we are moving now
	moveTimeStamp = g_timeSec;
	
	// Check if speed really differs
	if( speed == g_actualSpeed )
		return;	
	
	else if( speed > 0 )
	{
		int16_t ocr0 = ( ( ( int16_t ) FREF ) / speed ) - 1;
		OCR0 = ocr0 < 255 ? ocr0 : 255;
		g_actualDirection = 1;
		
		if( g_actualSpeed <= 0 ) // Handle speed direction change
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
		int16_t ocr0 = ( ( ( int16_t ) FREF ) / -speed ) - 1;
		OCR0 = ocr0 < 255 ? ocr0 : 255;
		g_actualDirection = -1;
		
		if( g_actualSpeed >= 0 )	// Handle direction change
		{
			TCCR0 |= CLOCK0_SELECT; 
			portbState = PORTB;
			portbState &= 0xff ^ _BV(PB2);	// Clear PB2 (dir)
			portbState &= 0xff ^ _BV(PB1);	// Clear PB1 (NENABLE)
			PORTB = portbState;			
		}
	}
	
	g_actualSpeed = speed;
}

#define TIME_MIN ( 1000 / PULSE_DURATION_SCALE )
#define TIME_MID ( 1500 / PULSE_DURATION_SCALE )
#define TIME_MAX ( 2000 / PULSE_DURATION_SCALE )
#define DEADBAND ( 300 / PULSE_DURATION_SCALE )
// 1..128
#define POWER_MAX 60

void processPulse( uint16_t pulseMs )
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
		g_speed = 0;
	}
	else
	{
		if( pulseMs > TIME_MAX )
			pulseMs = TIME_MAX;
		if( pulseMs < TIME_MIN )
			pulseMs = TIME_MIN;
			
		if( pulseMs >= ( TIME_MID + DEADBAND / 2 ) )
		{
			// Positive rotation.
			diff = ( pulseMs - ( TIME_MID + DEADBAND / 2 ) ) * POWER_MAX;
			diff /= ( TIME_MAX - ( TIME_MID + DEADBAND / 2 ) );
			if( diff > 127 )
				diff = 127;
				
			g_speed = diff;
		}
		else if( pulseMs <= ( TIME_MID - DEADBAND / 2 ) )
		{
			// Negative rotation.
			diff = ( ( TIME_MID - DEADBAND / 2 ) - pulseMs ) * POWER_MAX;
			diff /= ( TIME_MID - DEADBAND / 2 ) - TIME_MIN;
			if( diff > 127 )
				diff = 127;
				
			g_speed = -diff;
		}
		else
		{
			// In dead band, stop the motor.
			g_speed = 0;
		}
	}	
}

int16_t ucr0;

int main(void)
{
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
			/*_BV( CS02 ) | _BV( CS01 ) | _BV( CS00 ) | // clock / 64 */
			_BV( COM00 );								// Set OC0 on compare match when up-counting. Clear OC0 on compare match when down counting
	
	// Input -----------------------------------------------------------------
	// Capture source = ICP1 = PD4	
	// Whole D as input
	DDRD = 0x00;
	
	TCCR1B = _BV( ICNC1 ) |	// Allow noise canceler
			 _BV( CS11 )  |	// Clock / 8
			 _BV( ICES1 );	// A rising edge is capture event.
			 

	// Interrupt generator ----------------------------------------------------
	TCCR2 = _BV( WGM21 ) |				// CTC
			_BV( COM20 ) |				// Toggle OC2
			_BV( CS22 );				// Clock / 256
			
	// Interrupts -------------------------------------------------------------
	TIMSK = _BV( TOIE1 )  |	// Enable interrupt by timer1 overflow
			_BV( TICIE1 ) | // Enable interrupt by timer1 capture event
			_BV( OCIE0 )  |	// Enable interrupt by timer0 output compare match
			_BV( OCIE2 );	// Enable interrupt by timer2 output compare match
	
	TIFR = 0;				// Clear flags
	
	// I2C --------------------------------------------------------------------
	i2c_init();
	
	// Main loop --------------------------------------------------------------
	sei();
	
	OCR2 = 156;	// Approx 50 Hz.
	
	// USART
	mystdout.put = uart_putchar;
	mystdout.get = uart_getchar;
	mystdout.flags = _FDEV_SETUP_RW;
	stdin = stdout = &mystdout;
	DDRE = 0x01; // PE0 = RxD = input, PE1 = TxD = output

	usart_init( 9600, F_CPU );	
	//---------------------
	
	HMC5883L_init();
	
	
    while(1)
    {
		_delay_ms( 1000 );
		ucr0 = OCR0;

		printf( "%d %d %d %d\n", g_pulseDuration * PULSE_DURATION_SCALE, g_speed, g_actualSpeed, ucr0 );
		
		HMC5883L_read();		
    }

}


void HMC5883L_init(void)
{
	#define HMC5883L_WRITE 0x3C // write address
	#define HMC5883L_READ 0x3D // read address
	
	//	char buf[16];
	
	i2c_start(HMC5883L_WRITE);
	i2c_write(0x00); // set pointer to CRA
	i2c_write(0b01110100); //
	//	i2c_stop();
	//	i2c_start(HMC5883L_WRITE);
	//	i2c_write(0x01); // set pointer to CRB
	i2c_write(0b00000000);
	//	i2c_stop();
	//	i2c_start(HMC5883L_WRITE);
	//	i2c_write(0x02); // set pointer to measurement mode
	i2c_write(0x00); // continuous measurement
	i2c_stop();	
}

int16_t magx = 0, magy = 0, magz = 0;

void HMC5883L_read(void)
{
	#define HMC5883L_WRITE 0x3C // write address
	#define HMC5883L_READ 0x3D // read address
	
//	char buf[16];
	

	i2c_start(HMC5883L_WRITE);
	i2c_write(0x03); //set pointer to X-axis MSB
	i2c_stop();
		
	i2c_start(HMC5883L_READ);
		
	magx = i2c_read_ack() << 8;
	magx |= i2c_read_ack();
	magy = i2c_read_ack() << 8;
	magy |= i2c_read_ack();
	magz = i2c_read_ack() << 8;
	magz |= i2c_read_nack();

	i2c_stop();
}

