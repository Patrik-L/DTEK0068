/*
* File:   main.c
* Author: Patrik Larsen <pslars@utu.fi>
* Exercise: W06E01 - Digit Display
* Description: This project implements FreeRTOS and uses three different
* tasks to implement serial communication and the ability to change the
* value of the seven-segment display from a host device
*
* Created on December 04, 2021
*/

#include <avr/io.h> 
#include "FreeRTOS.h" 
#include "clock_config.h" 
#include "task.h" 
#include"queue.h"
#include <string.h>
#include <stdio.h>
#include "adc.h"
#include "lcd.h"

// Defining constants that are required for serial communication
#define F_CPU 3333333
#define USART0_BAUD_RATE(BAUD_RATE) \
((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)


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
    
   char final_message[800];

   sprintf(final_message, format_string, value);
   strcat(final_message, " ");
   
   usart0_send_string(final_message);
}

// Task that writes the values from message_queue to serial
void log_values(void* parameter)
{
    // start-up delay
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    // Infinite loop
    for (;;) 
    { 
        
        uint16_t ldr = read_adc(ADC_MUXPOS_AIN8_gc);
        uint16_t ntc = read_adc(ADC_MUXPOS_AIN9_gc);
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);
        
        log_value("LDR: (%d)", ldr);
        log_value("NTC: (%d)", ntc);
        log_value("POT: (%d)", pot);
        log_value("\r\n", 0);
  
       char display_text[30];
       
       lcd_cursor_set(0,0);
       sprintf(display_text, "LDR value: %d", ldr);
       lcd_write(display_text);
       vTaskDelay(660 / portTICK_PERIOD_MS);
       
       lcd_cursor_set(0,0);
       sprintf(display_text, "NTC value: %d", ntc);
       lcd_write(display_text);
       vTaskDelay(660 / portTICK_PERIOD_MS);
       
       lcd_cursor_set(0,0);
       sprintf(display_text, "POT value: %d", pot);
       lcd_write(display_text);
       vTaskDelay(660 / portTICK_PERIOD_MS);
        
        
    }
    // vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
}
  
  
int main(void) 
{ 

    
    adc_init();
    
    
    xTaskCreate( 
        lcd_init,
        "init_lcd",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    );
    
    // Setting up UART
    // Taken  from microchip documentation
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    USART0.CTRLB |= USART_TXEN_bm;
    USART0.CTRLB |= USART_RXEN_bm;
    
    // Creating the task that is used for outputting messages to serial
    xTaskCreate( 
        log_values,
        "log_values",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    );

    // Start the scheduler 
    vTaskStartScheduler(); 

    // Scheduler will not return 
    return 0; 
}