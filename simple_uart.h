#ifndef SIMPLE_UART_H
#define SIMPLE_UART_H

#include <stdint.h>

void	usart_init( uint32_t baudrate, uint32_t fCPU );
uint8_t usart_receive(void);
void	usart_send( uint8_t data );

#endif


