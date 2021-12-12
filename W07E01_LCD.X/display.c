
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h" 
#include "adc.h"
#include "lcd.h"
#include "task.h" 


void display_values(void* parameter)
{
    // start-up delay
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    lcd_init();
    
    // Infinite loop
    for (;;) 
    { 
        uint16_t ldr_val = read_adc(ADC_MUXPOS_AIN8_gc);
        uint16_t ntc_val = read_adc(ADC_MUXPOS_AIN9_gc);
        uint16_t pot_val = read_adc(ADC_MUXPOS_AIN14_gc);

        char display_text[sizeof(char) * 16];

        sprintf(display_text, "LDR value: %d", ldr_val);
        lcd_cursor_set(0,0);
        lcd_write(display_text);
        vTaskDelay(660 / portTICK_PERIOD_MS);
;
        sprintf(display_text, "NTC value: %d", ntc_val);
        lcd_cursor_set(0,0);
        lcd_write(display_text);
        vTaskDelay(660 / portTICK_PERIOD_MS);

        sprintf(display_text, "POT value: %d", pot_val);
        lcd_cursor_set(0,0);
        lcd_write(display_text);
        vTaskDelay(660 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL); 
}