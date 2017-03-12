#include "simple_uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void usart_init( uint32_t baudrate, uint32_t fCPU )
{
	uint32_t baudPrescaller = ( ( fCPU / ( baudrate * 16 ) ) ) - 1;
	
	 UBRR0H = (uint8_t)(baudPrescaller>>8);
	 UBRR0L = (baudPrescaller);
	 UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	 UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // 8-bit, 1 stop, no parity
}

uint8_t usart_receive(void)
{
	while(!(UCSR0A & _BV(RXC0)));
	return UDR0;
}

void usart_send( uint8_t data )
{
	while(!(UCSR0A & _BV(UDRE0)));
	UDR0 = data;
}