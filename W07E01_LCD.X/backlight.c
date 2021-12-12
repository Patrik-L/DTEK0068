

#include <avr/io.h>
#include "FreeRTOS.h" 
#include "adc.h"
#include "uart.h"
#include "timers.h"


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
    
    backlight_time = xTimerCreate
      ( /* Just a text name, not used by the RTOS
        kernel. */
        "Backlight",
        /* The timer period in ticks, must be
        greater than 0. */
        100,
        /* The timers will auto-reload themselves
        when they expire. */
        pdTRUE,
        /* The ID is used to store a count of the
        number of times the timer has expired, which
        is initialised to 0. */
        ( void * ) 3,
        /* Each timer calls the same callback when
        it expires. */
        backlight_timer_callback);
    TimerHandle_t timeout_time = xTimerCreate
      ( /* Just a text name, not used by the RTOS
        kernel. */
        "timeout",
        /* The timer period in ticks, must be
        greater than 0. */
        10000,
        /* The timers will auto-reload themselves
        when they expire. */
        pdFALSE,
        /* The ID is used to store a count of the
        number of times the timer has expired, which
        is initialised to 0. */
        ( void * ) 4,
        /* Each timer calls the same callback when
        it expires. */
        timeout_timer_callback);
    xTimerStart(backlight_time, 0);
    
    vTaskDelay(200);
    for(;;)
    {
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);
        if(last_pot == pot)
        {
            if(xTimerIsTimerActive(timeout_time) == pdFALSE)
            {
                xTimerStart(timeout_time, 0);
            }
        }
        else{
            if(xTimerIsTimerActive(backlight_time) == pdFALSE)
            {
                xTimerStart(backlight_time, 0);
            }
            if(xTimerIsTimerActive(timeout_time) == pdTRUE)
            {
                xTimerStop(timeout_time,0);
            }
            last_pot = pot;
        }
    }
    vTaskDelete(NULL);
}


