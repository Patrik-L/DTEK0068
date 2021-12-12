#include <stdio.h>
#include "FreeRTOS.h" 
#include "task.h"
#include "adc.h"
#include "task.h" 

#define LCD_WIDTH 16



void dummy(void* parameter)
{
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    PORTF.DIRSET = PIN5_bm;
    for (;;) 
    {
        uint16_t ntc = read_adc(ADC_MUXPOS_AIN9_gc);
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);
        
        if(ntc > pot){
            PORTF.OUTTGL = PIN5_bm;
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL); 
}