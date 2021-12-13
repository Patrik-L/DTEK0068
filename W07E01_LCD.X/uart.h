/* 
 * File:   uart.h
 * Author: patrik
 *
 * Created on December 11, 2021, 4:52 PM
 */

#ifndef UART_H
#define	UART_H

// Defining constants that are required for serial communication
#define F_CPU 3333333
#define USART0_BAUD_RATE(BAUD_RATE) \
((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

void log_value(char *format_string, uint16_t value);
void log_values(void* parameter);

#endif	/* UART_H */

