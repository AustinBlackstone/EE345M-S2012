/**
Filename:       adc.h
Name:           Cruz Monrreal II, Austin Blackstone
Creation Date:  01/30/2012
Lab #:          1
TA:             Zahidul
Last Revision:  10/30/2012
Description:    Helper functions for Timer-based ADC operation
*/
#ifndef __ADC_H
#define __ADC_H 1
#define ADC_IDLE 0
#define ADC_BUSY 1

// ADC Range: 0 to 1023
#define ADC_SAMPLE_NOT_READY 1024

// ADC Interrupt Handler
void ADC0IntHandler(void);

// Initialize ADC w/ Timer0
void ADC_Init(void);

// Perform ADC on Channel
unsigned short ADC_Read(unsigned int channelNum);

// Collect multiple samples from single ADC Channel
void ADC_Collect(unsigned int channelNum, unsigned int freq, void (*func)(unsigned short), unsigned int samples);

#endif
