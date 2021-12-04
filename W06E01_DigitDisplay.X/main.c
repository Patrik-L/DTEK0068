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

// Defining constants that are required for serial communication
#define F_CPU 3333333
#define USART0_BAUD_RATE(BAUD_RATE) \
((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

// Constants for num_queue and message_queue
static const uint8_t num_queue_len = 5;
static QueueHandle_t num_queue;

static const uint8_t message_max_length = 20;
static QueueHandle_t message_queue;

void drive_led(void* parameter) 
{ 
    // Array of LED states used to display numbers from 0-9 + E for 10.
    uint8_t segment_numbers[] =
    {
        0b00111111, 0b00000110, 0b01011011, 
        0b01001111, 0b01100110, 0b01101101, 
        0b01111101, 0b00000111, 0b01111111, 
        0b01100111, 0b01111001
    };

    // All 7-segment LED pins set as output
    VPORTC.DIR = 0xFF;

    // Turning on 7-segment LEDs
    PORTF.DIRSET = PIN5_bm;
    PORTF.OUTSET = PIN5_bm;

    uint8_t num_to_display = 0;

    // This task will run indefinitely 
    for (;;) 
    {
        // Continuously listening to num_queue
        // and displaying the number on the display
        if(xQueueReceive(num_queue, (void *) &num_to_display, 0) == pdTRUE)
        {
            VPORTC.OUT = segment_numbers[num_to_display];
        }
    }

    // vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
}

// Task that reads serial and sends appropriate
// messages to message_queue and num_queue
void uart_read(void* parameter)
{
    // Function that reads each character received.
    // Taken directly from microchips documentation.
    uint8_t usart0_read_char()
    {
        while (!(USART0.STATUS & USART_RXCIF_bm))
        {
            ;
        }
        return USART0.RXDATAL;
    }

    // Infinite loop
    for (;;) 
    { 
        uint8_t read_char = usart0_read_char();
        uint8_t segment_num;
        
        // If the character is a number between 0-9
        // 48 is a standard offset used in ASCII to get
        // the value of the number characters
        if((read_char >= 48) && (read_char <= 48+9))
        {
            segment_num = read_char - 48;    
            
            // Success message, \r\n signifies line break and then moving
            // the cursor to the beginning of the line
            const char *success_message="That's a valid digit!\r\n";
            xQueueSend(message_queue, (void *) &success_message, 10);

            xQueueSend(num_queue, (void *) &segment_num, 10);
        } else // If we've inputted a character that is not 0-9
        {
            // Error message, \r\n signifies line break and then moving
            // the cursor to the beginning of the line
            const char *error_message="Error! Not a valid digit.\r\n";
            xQueueSend(message_queue, (void *) &error_message, 10);

            segment_num = 10;
            xQueueSend(num_queue, (void *) &segment_num, 10);
        }
    }
    // vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
}

// Task that writes the values from message_queue to serial
void uart_write(void* parameter)
{
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
    
    char *message_string = "";
    
    // Infinite loop
    for (;;) 
    { 
        if(xQueueReceive(message_queue, (void *) &message_string, 0) == pdTRUE)
        {
            usart0_send_string(message_string);
        }
    }
    // vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
}
  
  
int main(void) 
{ 
    
    // Creating number queue, which is in charge of passing numbers
    // to the 7-segment display
    num_queue = xQueueCreate(num_queue_len, sizeof(uint8_t));
    
    // Creating message queue, which is in charge of sending messages
    // to serial
    message_queue = xQueueCreate(message_max_length, sizeof(char));
    

    // Creating the task that is used for updating the 7-segment display 
    xTaskCreate( 
        drive_led, 
        "drive_led", 
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

    // Creating the task that is used for reading serial
    // and sending the appropriate messages to message_queue and num_queue
    xTaskCreate( 
        uart_read, 
        "uart_read", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY, 
        NULL 
    );
    
    // Creating the task that is used for outputting messages to serial
    xTaskCreate( 
        uart_write,
        "uart_write",
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