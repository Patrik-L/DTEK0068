
#include <avr/io.h> 
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h" 
#include "task.h"

void log_value(char *format_string, uint16_t value){
    // Sends a single character to serial.
    // Taken directly from microchip documentation
    void usart0_send_char(char c)
    {
        while (!(USART0.STATUS & USART_DREIF_bm))
        {
            ;
        }
        USART0.TXDATAL = c;
    }
    
    // Sends string to serial using sendChar.
    // Taken directly from microchip documentation
    void usart0_send_string(char *str)
    {
        for(size_t i = 0; i < strlen(str); i++)
        {
            usart0_send_char(str[i]);
        }
    }
    
    
    char final_message[14];

   snprintf(final_message,14, format_string, value);
   
   usart0_send_string(final_message);
}