

#include <avr/io.h>
#include "FreeRTOS.h" 
#include "adc.h"
#include "uart.h"
#include "timers.h"
#include "task.h"

uint16_t last_pot;
TimerHandle_t backlight_time;

void backlight_timer_callback()
{
    uint16_t ratio = read_adc(ADC_MUXPOS_AIN8_gc)*60;
    TCB3.CCMP = ratio;
}
void timeout_timer_callback()
{
    if(xTimerIsTimerActive(backlight_time) == pdTRUE)
    {
        xTimerStop(backlight_time, 0);
    }
    TCB3.CCMP = 0;
}


void TCB3_init (void)
{
    
    /* Load CCMP register with the period and duty cycle of the PWM */
    TCB3.CCMP = 0xffff;

    /* Enable TCB3 and Divide CLK_PER by 2 */
    TCB3.CTRLA |= TCB_ENABLE_bm;
    TCB3.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
    
    /* Enable Pin Output and configure TCB in 8-bit PWM mode */
    TCB3.CTRLB |= TCB_CCMPEN_bm;
    TCB3.CTRLB |= TCB_CNTMODE_PWM8_gc;
}

/*
void backlight_adjuster(void* parameter)
{
    // Infinite loop
    for (;;) 
    {
        uint16_t ldr = read_adc(ADC_MUXPOS_AIN8_gc);
        
        double ratio = ldr/1024;
        
        TCB3.CCMP = (ldr*60);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL); 
}
 */

void backlight_init()
{
    PORTB.DIRSET = PIN5_bm;
    PORTB.OUTSET = PIN5_bm;
}

void backlight_adjuster(void *param)
{
    vTaskDelay(200 / portTICK_PERIOD_MS);
    uint8_t tick_count = 0;
    uint16_t last_pot = 0;
    uint16_t display_brightness = 0xFFFF;
    
    for(;;)
    {
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);
        
        if((last_pot <= pot+50 && last_pot >= pot-50)){
            tick_count += 1;
        } else {
            tick_count = 0;
        }
        
        if(tick_count > 100){
            tick_count = 100;
        }
        
        last_pot = pot;
        
        
        if(tick_count >= 100){
            display_brightness = 0;
        } else {
            display_brightness = read_adc(ADC_MUXPOS_AIN8_gc)*60;
        }
        
        //log_value("TIK: %04d\r\n", tick_count);
        
        TCB3.CCMP = display_brightness;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


