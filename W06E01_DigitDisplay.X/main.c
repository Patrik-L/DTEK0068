/*
 * File:   main.c
 * Author: Patrik Larsen <pslars@utu.fi>
 * Exercise: W04E01 - Dino Player
 * Description: Program to automatically play the dino game accessible
 * on chromium based browsers at "chrome://dino".
 * An onboard potentiometer is used to fine-tune the activation threshold
 * of a photoresistor. Said photoresistor is to be placed over the monitor
 * to detect the dark pixels of approaching cacti.
 *
 * Created on November 16, 2021
 */
    
  /* 
  * Minimal FreeRTOS application - Surprise! It blinks a LED! 
  */  
 #include <avr/io.h> 
 #include "FreeRTOS.h" 
 #include "clock_config.h" 
 #include "task.h" 
 #include"queue.h"
 #include <string.h>

#define F_CPU 3333333
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)
  
 #define BLINK_DELAY    0  // milliseconds 

static const uint8_t num_queue_len = 5;
static QueueHandle_t num_queue;

static const uint8_t message_max_length = 20;
static QueueHandle_t message_queue;


 void DriveLED(void* parameter) 
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
     
    // Turning on 7-segment LED
    PORTF.DIRSET = PIN5_bm;
    PORTF.OUTSET = PIN5_bm;
       
     
     
     PORTF.DIRSET = PIN5_bm;                      // PF5 (LED) as output 
     
     uint8_t num_to_display = 0;
     // This task will run indefinitely 
     for (;;) 
     {
         if(xQueueReceive(num_queue, (void *) &num_to_display, 0) == pdTRUE);
         {
             VPORTC.OUT = segment_numbers[num_to_display];  
         }
         vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));  // Wait n milliseconds 
     } 
     // Above loop will not end, but as a practice, tasks should always include 
     // a vTaskDelete() call just-in-case 
     vTaskDelete(NULL); 
 }

  void UARTRead(void* parameter)
 {
    uint8_t USART0_readChar()
    {
        while (!(USART0.STATUS & USART_RXCIF_bm))
        {
            ;
        }
        return USART0.RXDATAL;
    }
    
    for (;;) 
     { 
        uint8_t read_char = USART0_readChar();
        
        
        //Send Num
        uint8_t num;
        if(read_char >= (48) && read_char <= (48+9)){
            num = read_char - 48;    
            
            const char *success_message="That is indeed a digit!\r\n";
            xQueueSend(message_queue, (void *) &success_message, 10);
            
            xQueueSend(num_queue, (void *) &num, 10);
        } else
        {
            const char *error_message="Error! Not a valid digit.\r\n";
            xQueueSend(message_queue, (void *) &error_message, 10);
            
            num = 10;
            xQueueSend(num_queue, (void *) &num, 10);
        }
        
        
        vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));  // Wait n milliseconds 
     }
    // Above loop will not end, but as a practice, tasks should always include 
    // a vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
 }
  
    void UARTWrite(void* parameter)
 {
      
    void USART0_sendChar(char c)
    {
        while (!(USART0.STATUS & USART_DREIF_bm))
        {
            ;
        }
        USART0.TXDATAL = c;
    }
    
    void USART0_sendString(char *str)
    {
        for(size_t i = 0; i < strlen(str); i++)
        {
            USART0_sendChar(str[i]);
        }
    }
    
    char *message_string = " ";
    
    for (;;) 
     { 
        if(xQueueReceive(message_queue, (void *) &message_string, 0) == pdTRUE)
         {
            USART0_sendString(message_string);
         }
         vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));  // Wait n milliseconds 
     }
    // Above loop will not end, but as a practice, tasks should always include 
    // a vTaskDelete() call just-in-case 
    vTaskDelete(NULL); 
 }
  
  
 int main(void) 
 { 
     
     // Creating number queue
     num_queue = xQueueCreate(num_queue_len, sizeof(uint8_t));
     
     // Creating queue
     message_queue = xQueueCreate(message_max_length, sizeof(char)); //20 char max
     
   
     // Create task 
     xTaskCreate( 
         DriveLED, 
         "drive_led", 
         configMINIMAL_STACK_SIZE, 
         NULL, 
         tskIDLE_PRIORITY, 
         NULL 
     );
     
    //setting up UART
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    USART0.CTRLB |= USART_TXEN_bm;
    USART0.CTRLB |= USART_RXEN_bm;
     
     // Create task 
     xTaskCreate( 
         UARTRead, 
         "UART_read", 
         configMINIMAL_STACK_SIZE, 
         NULL, 
         tskIDLE_PRIORITY, 
         NULL 
     );
     
          // Create task 
     xTaskCreate( 
         UARTWrite, 
         "UART_write", 
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