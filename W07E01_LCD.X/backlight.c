

#include <avr/io.h>
#include "FreeRTOS.h" 
#include "adc.h"
#include "uart.h"
#include "timers.h"
#include "task.h"

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

void backlight_init()
{
    PORTB.DIRSET = PIN5_bm;
    PORTB.OUTSET = PIN5_bm;
}

void backlight_adjuster(void *param)
{
  // start-up delay
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    uint8_t tick_count = 0;
    uint16_t last_pot = 0;
    uint16_t display_brightness = 0xFFFF;
    
    for(;;)
    {
        uint16_t pot = read_adc(ADC_MUXPOS_AIN14_gc);
        
        // If potentiometer has not been moved we add a tick,
        // I'm using a range check, since in
        // my experience the pot values jumped around a fair bit
        if((last_pot <= pot+50 && last_pot >= pot-50))
        {
            tick_count += 1;
        } else
        {
            // We reset the tick count if we have moved the potentiometer
            tick_count = 0;
        }
        
        // Preventing overflow
        if(tick_count > 100)
        {
            tick_count = 100;
        }
        
        last_pot = pot;
        
        // if 100 ticks (100ms * 100 = 10s) have passed)
        if(tick_count >= 100)
        {
            // Setting brightness to 0, shutting down the back light.
            display_brightness = 0;
        } else
        {
            // Setting brightness to 60 times the adc value so that
            // we can get a larger range of brightness adjustment.
            display_brightness = read_adc(ADC_MUXPOS_AIN8_gc)*60;
        }
        
        // Setting the actual screen brightness
        TCB3.CCMP = display_brightness;
        // updating screen every tenth of a second.
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // Just in case
    vTaskDelete(NULL);
}


