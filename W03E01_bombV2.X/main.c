/*
 * File:   main.c
 * Author: Patrik Larsen <pslars@utu.fi>
 * Exercise: W03E01 - BombV2
 * Description: 7-segment display, that counts down to 0.
 * When wire at PA4 is disconnected, the countdown is stopped.
 * If timer reaches 0 display starts blinking.
 * 
 * The code has been re-factored to make use of timers and sleep.
 * 
 * Note: This program uses the same wiring as last weeks submission, with
 * the exception of the added transistor.
 *
 * Created on November 11, 2021
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>  // for ccp_write_io()
#include <avr/sleep.h> 

// Global variable tracking that red wire hasn't been disconnected
uint8_t volatile g_running = 1;

// Global variable expressing how many seconds have elapsed
uint8_t volatile g_clockticks = 0;

void rtc_init(void)
{
    uint8_t temp;
    
    // Disable oscillator 
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_ENABLE_bm;
    ccp_write_io((void*) &CLKCTRL.XOSC32KCTRLA, temp);
    
    // Wait for the clock to be released (0 = unstable, unused) 
    while (CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm);
    
    // Select external crystal (SEL = 0) 
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_SEL_bm;
    ccp_write_io((void*) &CLKCTRL.XOSC32KCTRLA, temp);
    
    // Enable oscillator 
    temp = CLKCTRL.XOSC32KCTRLA;
    temp |= CLKCTRL_ENABLE_bm;
    ccp_write_io((void*) &CLKCTRL.XOSC32KCTRLA, temp);
    
    // Wait for the clock to stabilize 
    while (RTC.STATUS > 0);
    
    // Configure RTC module 
    // Select 32.768 kHz external oscillator 
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;
    
    // Enable Periodic Interrupt 
    RTC.PITINTCTRL = RTC_PI_bm;
    
    //Set period to 4096 cycles (1/8 second) and enable PIT function 
    RTC.PITCTRLA = RTC_PERIOD_CYC4096_gc | RTC_PITEN_bm;
}

int main(void)
{
    // All 7-segment LED pins set as output
    VPORTC.DIR = 0xFF;
    
    // Turning on 7-segment LED
    PORTF.DIRSET = PIN5_bm;
    PORTF.OUTSET = PIN5_bm;
    
    // Setting red wire as input
    PORTA.DIRCLR = PIN4_bm;
    
    // Trigger interrupts on rising edge on the red wire pin and enable
    // pull up resistor
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    
    // Initialize RTC timer's PIT interrupt function
    rtc_init();
    
    // Array of LED states used to display numbers from 0-9.
    uint8_t segment_numbers[] =
    {
        0b00111111, 0b00000110, 0b01011011, 
        0b01001111, 0b01100110, 0b01101101, 
        0b01111101, 0b00000111, 0b01111111, 
        0b01100111
    };
    
    // Enabling interrupts
    sei();
    
    uint8_t number_to_display;
    while(1)
    {
        // Displaying remaining time, unless countdown has reached 0
        if(g_clockticks < 9)
        {
            number_to_display = 9 - g_clockticks;
        } else
        {
            number_to_display = 0;
        }
        if(g_running == 0)
        {
            cli(); // Disable interrupts, causing countdown to stop
        }
        
        // Update display with current number
        VPORTC.OUT = segment_numbers[number_to_display];
        
        // Sleep until next interrupt.
        sleep_mode();
    };
}

// Interrupt handler for when the red wire is disconnected.
ISR(PORTA_PORT_vect) 
{ 
    // Clear interrupt flag
    VPORTA.INTFLAGS = PIN4_bm;
    
    // Only "defusing" bomb if countdown hasn't reached 0
    if(g_clockticks < 9)
    {
        g_running = 0;
    }
}

// Interrupt handler firing every clock-tick
ISR(RTC_PIT_vect) 
{ 
    // Clear interrupt flag 
    RTC.PITINTFLAGS = RTC_PI_bm; 

    static uint8_t PIT_count = 0;
    PIT_count += 1;
    
    // Increment g_clockticks every 8:th clock-tick.
    // We also stop incrementing g_clockticks if countdown reaches 0, since
    // it would otherwise overflow after 255 seconds
    if(PIT_count == 7 && g_clockticks < 9)
    {
        g_clockticks += 1;
        PIT_count = 0;
    }
    
    // Checks if 9 seconds have passed
    if(g_clockticks >= 9)
    {
        // Blinking 7-segment display
        PORTF.OUTTGL = PIN5_bm;
    }
}