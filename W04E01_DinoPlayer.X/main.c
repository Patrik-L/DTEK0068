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


#include <avr/io.h>
#include <avr/cpufunc.h>  // for ccp_write_io()
#include <avr/interrupt.h>

// Servo PWM constants
#define SERVO_PWM_PERIOD        (0x1046) 
#define SERVO_PWM_DUTY_NEUTRAL  (0x0138) 
#define SERVO_PWM_DUTY_MAX      (0x01A0) 

// Used to inform ISR that we want to press space bar 
uint8_t volatile g_pressing = 0;

// Used to inform ISR that the servo is returning from a press
uint8_t volatile g_returning = 0;

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
    
    // Set period to  Enable OVF interrupt8192 cycles (1/8 second)
    RTC.PER = 4096;
    
    // Configure RTC module 
    // Select 32.768 kHz external oscillator 
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;
    RTC.INTCTRL |= RTC_OVF_bm; // Enable OVF interrupt
    RTC.CTRLA = RTC_RTCEN_bm; //Enable RTC
}

void press_spacebar()
{ 
    // Tell ISR that the next cycle should be a press event
    g_pressing = 1;
    
    // Resetting RTC, so that press happens after 1/8 second
    RTC.PER = 4096;
}


int main(void)
{
    
    // Enabling interrupts
    sei();
    
    // All 7-segment LED pins set as output
    VPORTC.DIR = 0xFF;
    
    // Turning on 7-segment LED
    PORTF.DIRSET = PIN5_bm;
    PORTF.OUTSET = PIN5_bm;
    
    // potentiometer
    // Set potentiometer as input 
    PORTF.DIRCLR = PIN4_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    // Enable (power up) ADC (10-bit resolution is default) 
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    // ldr
    // Set ldr as input 
    PORTE.DIRCLR = PIN0_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTE.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    
    // Setting internal voltage reference that is used with ldr
    VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc; // Set internal voltage ref to 2.5V
    
    //Servo 
    // Route TCA0 PWM waveform to PORTB 
    PORTMUX.TCAROUTEA |= PORTMUX_TCA0_PORTB_gc; 
    // Set 0-WO2 (PB2) as digital output 
    PORTB.DIRSET = PIN2_bm; 
    // Set TCA0 prescaler value to 16 (~208 kHz) 
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc; 
    // Set single-slop PWM generation mode 
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; 
    // Using double-buffering, set PWM period (20 ms) 
    TCA0.SINGLE.PERBUF = SERVO_PWM_PERIOD; 
    // Set initial servo arm position as neutral (0 deg) 
    TCA0.SINGLE.CMP2BUF = SERVO_PWM_DUTY_NEUTRAL; 
    // Enable Compare Channel 2 
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm; 
    // Enable TCA0 peripheral 
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; 

    // Array of LED states used to display numbers from 0-9 + A for 10.
    uint8_t segment_numbers[] =
    {
        0b00111111, 0b00000110, 0b01011011, 
        0b01001111, 0b01100110, 0b01101101, 
        0b01111101, 0b00000111, 0b01111111, 
        0b01100111, 0b01110111
    };
    
    // Initialize RTC timer
    rtc_init();
    
    uint16_t pot_reading = 0;
    uint16_t ldr_reading = 0; 
    while(1)
    {

        change_mux(ADC_MUXPOS_AIN8_gc);
        ldr_reading = ADC0.RES/100; // Getting value of photoresistor
        
        change_mux(ADC_MUXPOS_AIN14_gc);
        pot_reading = ADC0.RES/100; // Getting value of potentiometer
               
        uint8_t number_to_display = 10;   
        
        // Clamping reading to 10
        if(pot_reading <= 10)
        {
            number_to_display = pot_reading;
        };

        // If threshold has been met, issue press
        if(ldr_reading > pot_reading)
        {
            press_spacebar();
        } 
             
        VPORTC.OUT = segment_numbers[number_to_display]; 
        ADC0.INTFLAGS = ADC_RESRDY_bm; 
    };
}

ISR(RTC_CNT_vect)
{ 
    // Setting INTFLAGS
	RTC.INTFLAGS |= RTC_OVF_bm;
    
    // Check if press has been requested and that we aren't returning
    if(g_pressing && !g_returning)
    {
        // Sets servo to move to max position
        TCA0.SINGLE.CMP2BUF = SERVO_PWM_DUTY_MAX;
        
        // Informs that next RTC event should be a return
        g_returning = 1;
        
        // Press event has completed when the next RTC event fires.
        g_pressing = 0;
    } else if (g_returning) // If we're in the pressed position
    {
        // Sets servo to neutral position
        TCA0.SINGLE.CMP2BUF = SERVO_PWM_DUTY_NEUTRAL; 
        
        // Return event has completed when the next RTC event fires.
        g_returning = 0;
    }   
}