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
#include "adc.h"
#include "uart.h"
#include "display.h"
#include "lcd.h"
#include "backlight.h"
#include "scroller.h"
#include "dummy.h"
  
int main(void) 
{ 
    // Setting up UART
    // Taken  from microchip documentation
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    USART0.CTRLB |= USART_TXEN_bm;
    USART0.CTRLB |= USART_RXEN_bm;
    
    adc_init();
    backlight_init();
    TCB3_init();
    
    // Starting the tasks
    
    xTaskCreate( 
        log_values,
        "log_values",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    );
    
 
    xTaskCreate( 
    display_values,
    "display_values",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY,
    NULL
    );
    
        
    xTaskCreate( 
        scroll,
        "scroll",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        NULL
    );
        
    xTaskCreate( 
        dummy,
        "dummy",
        configMINIMAL_STACK_SIZE,
        NULL,
        // Max possible priority
        configMAX_PRIORITIES - 1,
        NULL
    );
    
    
    xTaskCreate( 
        backlight_adjuster,
        "backlight_adjuster",
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