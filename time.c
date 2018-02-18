#include "time.h"
#include "fcpu.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )						// 8
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )		
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))	// 1024
// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)					// 1
// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)				// 3
#define FRACT_MAX (1000 >> 3)													// 125

void initTime(void)
{
	TCCR2 = _BV( CS21 ) | _BV( CS20 );	// clock/32
//	TIMSK = _BV(TOIE2);					// Enable overflow interrupt, set in main()
}

volatile uint32_t timer0_overflow_count = 0;
volatile uint32_t timer0_millis = 0;
static uint8_t timer0_fract = 0;

ISR(TIMER2_OVF_vect)
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	uint32_t m = timer0_millis;
	uint8_t f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) 
	{
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

uint32_t millis(void)
{
	uint32_t m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}
