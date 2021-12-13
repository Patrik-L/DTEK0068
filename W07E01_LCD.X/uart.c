
#include <avr/io.h> 
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h" 
#include "task.h"
#include "adc.h"

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
    
   
   // All of our serial strings are 14 characters long
   char final_message[14];
   snprintf(final_message,14, format_string, value);
   
   // Sending the formatted string.
   usart0_send_string(final_message);
}

void log_values(void* parameter)
{
    // start-up delay
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    // Infinite loop
    for (;;) 
    { 
        // getting sensor values.
        uint16_t ldr = read_adc(ADC_MUXPOS_AIN8_gc);
        uint16_t ntc = read_adc(ADC_MUXPOS_AIN9_gc);
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);

        // Printing the values.
        log_value("LDR: (%04d)  ", ldr);
        log_value("NTC: (%04d)  ", ntc);
        log_value("POT: (%04d)\r\n", pot);        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    }
    // vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
}