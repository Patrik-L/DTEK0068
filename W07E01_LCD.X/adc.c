
#include "FreeRTOS.h" 
#include "semphr.h"
#include "adc.h"

void adc_init()
{
    // Create mutex before starting tasks
    g_adc_mutex = xSemaphoreCreateMutex();
    
    VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc; // Set internal voltage ref to 2.5V
        //Read LDR value
    ADC0.CTRLC &= ~(ADC_REFSEL_VDDREF_gc); //Clear REFSEL bits
    //Voltage reference to internal 2.5V
    ADC0.CTRLC |= ADC_REFSEL_INTREF_gc;
        // Set prescaler of 16 
    ADC0.CTRLC |= ADC_PRESC_DIV16_gc; 
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    // potentiometer
    // Set potentiometer as input 
    PORTF.DIRCLR = PIN4_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    // Enable (power up) ADC (10-bit resolution is default) 
    ADC0.CTRLA |= ADC_ENABLE_bm;
}

uint16_t last_value = 0;

// Task that writes the values from message_queue to serial
uint16_t read_adc(register8_t muxpos)
{
   // if( xSemaphoreTake( g_adc_mutex, 100 ) == pdTRUE )
    //{
        ADC0.MUXPOS = muxpos;

        // Start conversion (bit cleared when conversion is done) 
        ADC0.COMMAND = ADC_STCONV_bm;

        // Waiting for adc to get a reading
        while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
        { 
            ;
        }
        
        last_value = ADC0.RES;
     //   xSemaphoreGive(g_adc_mutex);
        return last_value;
   // } else{
    //  return last_value; 
   //}
    
    
}

// Changes adc muxpos, sets correct voltage ref and
// waits for the conversion to finish.
// Expects only ADC_MUXPOS_AIN8_gc and ADC_PRESC_DIV16_gc as input
void change_mux(register8_t muxpos)
{
    ADC0.MUXPOS = muxpos;
    
    // Changing voltage ref depending on which port we're reading
    if(muxpos == ADC_MUXPOS_AIN8_gc)
    {
        ADC0.CTRLC |= ADC_PRESC_DIV16_gc |  ADC_REFSEL_INTREF_gc;
    } else
    {
        ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
    }

    // Start conversion (bit cleared when conversion is done) 
    ADC0.COMMAND = ADC_STCONV_bm;

    // Waiting for adc to get a reading
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
    { 
        ;
    }
    return;
}
