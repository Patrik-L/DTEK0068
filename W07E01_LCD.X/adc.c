
#include "FreeRTOS.h" 
#include "semphr.h"
#include "adc.h"

void adc_init()
{
    // Create mutex before starting tasks
    g_adc_mutex = xSemaphoreCreateMutex();
    
    ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
    
    // potentiometer
    // Set potentiometer as input 
    PORTF.DIRCLR = PIN4_bm; 
    // No pull-up, no invert, disable input buffer 
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc; 
    // Enable (power up) ADC (10-bit resolution is default) 
    ADC0.CTRLA |= ADC_ENABLE_bm;
}


// Task that writes the values from message_queue to serial
uint16_t read_adc(register8_t muxpos)
{   
    //xSemaphoreTake(g_adc_mutex, portMAX_DELAY);
    ADC0.MUXPOS = muxpos;

    // Start conversion (bit cleared when conversion is done) 
    ADC0.COMMAND = ADC_STCONV_bm;

    // Waiting for adc to get a reading
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
    { 
        ;
    }
    //xSemaphoreGive(g_adc_mutex);
    return ADC0.RES;
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