/*
 * File:   main.c
 * Author: Patrik Larsen
 *
 * Created on October 29, 2021, 6:52 PM
 */


#include <avr/io.h>
int main(void) {
    
    //LED as output
    PORTF.DIR = PORTF.DIR | PIN5_bm;
    
    //BUTTON as input
    PORTF.DIR = PORTF.DIR & ~PIN6_bm;

    while (1) {
        if (PORTF.IN & PIN6_bm){
            // If button is not pressed
            PORTF.OUT |= PIN5_bm; //led is turned off
        } else {
            // if the button is pressed
            PORTF.OUT &= ~PIN5_bm; //led is turned on
        }
    }
}
