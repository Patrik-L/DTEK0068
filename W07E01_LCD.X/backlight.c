

#include <avr/io.h>

void TCB3_init (void)
{
    /* Load CCMP register with the period and duty cycle of the PWM */
    TCB3.CCMP = 0x80FF;

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