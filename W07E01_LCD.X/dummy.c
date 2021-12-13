#include <stdio.h>
#include "FreeRTOS.h" 
#include "task.h"
#include "adc.h"
#include "task.h" 

#define LCD_WIDTH 16



void dummy(void* parameter)
{
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    // Setting internal led as an output
    PORTF.DIRSET = PIN5_bm;
    for (;;) 
    {
        uint16_t ntc = read_adc(ADC_MUXPOS_AIN9_gc);
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);
        
        // toggling led if ntc value is bigger than pot
        if(ntc > pot)
        {
            PORTF.OUTTGL = PIN5_bm;
        }
        // Just in case
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL); 
}