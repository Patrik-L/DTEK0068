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
#include <avr/cpufunc.h>  // for ccp_write_io()

#define F_CPU  3333333   // 3.33 MHz Clock frequency 
#include <util/delay.h>

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
    
    // Set PF4 as input 
    PORTF.DIRCLR = PIN4_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    // Enable (power up) ADC (10-bit resolution is default) 
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    
    //LIGHT
    // Set PE0 as input 
    PORTE.DIRCLR = PIN0_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTE.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    
    
    //Servo
    
    PORTB.DIRSET = PIN2_bm;
    
    
    // Array of LED states used to display numbers from 0-9 + A.
    uint8_t segment_numbers[] =
    {
        0b00111111, 0b00000110, 0b01011011, 
        0b01001111, 0b01100110, 0b01101101, 
        0b01111101, 0b00000111, 0b01111111, 
        0b01100111, 0b01110111
    };
    
    
    uint16_t pot_reading = 0;
    
    uint16_t ldr_reading = 0;
    
    uint8_t first_time = 1;
    
    while(1)
    {
        if(first_time == 1){
            ;
        }

        ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
        // Use Vdd as reference voltage and set prescaler of 16 
        ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
        
        // Start conversion (bit cleared when conversion is done) 
        ADC0.COMMAND = ADC_STCONV_bm;

        while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
        { 
            ;
        }
        
        ldr_reading = floor(ADC0.RES); 
        
        
        
        ADC0.MUXPOS = ADC_MUXPOS_AIN14_gc;
        ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
        
        // Start conversion (bit cleared when conversion is done) 
        ADC0.COMMAND = ADC_STCONV_bm;

        while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
        { 
            ;
        }
        
        pot_reading = floor(ADC0.RES / 10);   
            
          
          
         uint8_t number_to_display = 10;
         
         if(ldr_reading/pot_reading <= 10)
         {
             number_to_display = ldr_reading/pot_reading;
         };
         
         if(number_to_display == 10) {
             press_spacebar();
         } 
             
        VPORTC.OUT = segment_numbers[number_to_display]; 
        
        ADC0.INTFLAGS = ADC_RESRDY_bm; 
    };
}

void press_spacebar(){ 
    for(uint16_t i = 0; i<100; i++)
    {
         PORTB.OUTTGL = PIN2_bm;
         _delay_ms(1);
    }                                                                                                                                                                                                                                                                          
    for(uint16_t i = 0; i<100; i++)
    {
         PORTB.OUTTGL = PIN2_bm;
         _delay_ms(2);
    }
}
