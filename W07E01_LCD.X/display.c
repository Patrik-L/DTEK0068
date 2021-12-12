
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

        char display_text[16];

        snprintf(display_text, 16, "LDR value: %04d", ldr_val);
        lcd_cursor_set(0,0);
        lcd_write(display_text);
        vTaskDelay(660 / portTICK_PERIOD_MS);
;
        snprintf(display_text, 16, "NTC value: %04d", ntc_val);
        lcd_cursor_set(0,0);
        lcd_write(display_text);
        vTaskDelay(660 / portTICK_PERIOD_MS);

        snprintf(display_text, 16, "POT value: %04d", pot_val);
        lcd_cursor_set(0,0);
        lcd_write(display_text);
        vTaskDelay(660 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL); 
}