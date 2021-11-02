/*
 * File:   main.c
 * Author: Patrik Larsen <pslars@utu.fi>
 * Exercise: W02E01 - Bomb
 * Description: 7-segment display, that counts down to 0.
 * When wire at PA4 is disconnected, the countdown is stopped.
 * If timer reaches 0 display starts blinking.
 *
 * Created on November 2, 2021
 */


#include <avr/io.h>
#define F_CPU  3333333   // 3.33 MHz Clock frequency 
#include <util/delay.h>
#include <avr/interrupt.h>

// Global variable tracking that red wire hasn't been disconnected
uint8_t g_running = 1;

int main(void)
{
    // All 7-segment LED pins set as output
    VPORTC.DIR = 0xFF;
    
    // Setting red wire as input
    PORTA.DIRCLR = PIN4_bm;
    
    // Trigger interrupts on rising edge on the red wire pin
    PORTA.PIN4CTRL = PORT_ISC_RISING_gc; 
    
    // Array of LED states used to display numbers from 0-9,
    // additionally index 10 is used for the off-state.
    uint8_t segment_numbers[] =
    {
        0b00111111, 0b00000110, 0b01011011, 
        0b01001111, 0b01100110, 0b01101101, 
        0b01111101, 0b00000111, 0b01111111, 
        0b01100111, 0b00000000
    };
    
    // Enabling interrupts
    sei();
    
    // Displaying numbers from 9 to 1, with 1s delay
    for(uint8_t i = 9; i>0; i--)
    {
        // Displaying current number
        VPORTC.OUT = segment_numbers[i];
        _delay_ms(1000);
        
        // Halting the program if the red wire has been disconnected
        if(!g_running){
            return;
        }
    };
    
    // Infinite loop blinking 0
    while(1)
    {
        VPORTC.OUT = segment_numbers[0];
        _delay_ms(1000);
        VPORTC.OUT = segment_numbers[10]; // Display nothing
        _delay_ms(1000);
    };
}

// Interrupt handler for when the red wire is disconnected.
ISR(PORTA_PORT_vect) 
{ 
    // Setting INTFLAG even though it's pretty unnecessary in our case,
    // since the program will halt anyways
    VPORTA.INTFLAGS = PIN4_bm;
    g_running = 0;
}
