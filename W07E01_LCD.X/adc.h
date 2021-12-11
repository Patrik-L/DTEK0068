/* 
 * File:   adc.h
 * Author: patrik
 *
 * Created on December 11, 2021, 3:09 AM
 */

#ifndef ADC_H
#define	ADC_H

#include "semphr.h"


// Globals
SemaphoreHandle_t g_adc_mutex;

void adc_init(void);

uint16_t read_adc(register8_t muxpos);

void change_mux(register8_t muxpos);

#endif