/*
 * File:   main.c
 * Author: Patrik Larsen <pslars@utu.fi>
 * Exercise: W01E01 - Push-button Light
 * Description: Turns on and off the built-in light at the push
 * of a button.
 *
 * Created on October 29, 2021, 6:52 PM
 */


#include <avr/io.h>
int main(void) {

    // LED pin set as output
    PORTF.DIR = (PORTF.DIR | PIN5_bm);
    
    // Button pin set as input
    PORTF.DIR = (PORTF.DIR & ~PIN6_bm);

    while (1) {
        // If button is not pressed
        if (PORTF.IN & PIN6_bm)
        {
            PORTF.OUT |= PIN5_bm; // LED is turned off
        }
        // If the button is pressed
        else
        {
            PORTF.OUT &= ~PIN5_bm; // LED is turned on
        }
    }
}
