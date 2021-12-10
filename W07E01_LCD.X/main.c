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
#include "semphr.h"

// Defining constants that are required for serial communication
#define F_CPU 3333333
#define USART0_BAUD_RATE(BAUD_RATE) \
((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

// Globals
static SemaphoreHandle_t g_adc_mutex;


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
uint16_t read_adc(register8_t muxpos)
{   
    xSemaphoreTake(g_adc_mutex, portMAX_DELAY);
    ADC0.MUXPOS = muxpos;

    // Start conversion (bit cleared when conversion is done) 
    ADC0.COMMAND = ADC_STCONV_bm;

    // Waiting for adc to get a reading
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
    { 
        ;
    }
    xSemaphoreGive(g_adc_mutex);
    return ADC0.RES;
}


// Task that writes the values from message_queue to serial
void log_values(void* parameter)
{
    // start-up delay
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    // Infinite loop
    for (;;) 
    { 
        
        log_value("LDR: (%d)", read_adc(ADC_MUXPOS_AIN8_gc));
        log_value("NTC: (%d)", read_adc(ADC_MUXPOS_AIN9_gc));
        log_value("POT: (%d)", read_adc(ADC_MUXPOS_AIN14_gc));
        log_value("\r\n", 0);
     
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
}
  
  
int main(void) 
{ 
    
    // Setting up UART
    // Taken  from microchip documentation
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    USART0.CTRLB |= USART_TXEN_bm;
    USART0.CTRLB |= USART_RXEN_bm;
    
    ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
    
    // potentiometer
    // Set potentiometer as input 
    PORTF.DIRCLR = PIN4_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    // Enable (power up) ADC (10-bit resolution is default) 
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    
    // Create mutex before starting tasks
    g_adc_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(g_adc_mutex);
    
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