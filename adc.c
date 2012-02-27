/**
Filename:       adc.c
Name:           Cruz Monrreal II, Austin Blackstone
Creation Date:  01/25/2012
Lab #:          1
TA:             Zahidul
Last Revision:  10/30/2012
Description:    Helper functions for Timer-based ADC operation
*/

#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"

#include "lm3s1968.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "adc.h"
#include "os.h"

unsigned long adc_last_value;
unsigned int adc_samples;
unsigned char adc_status;
void (*function)(unsigned short);

extern int TIMELORD;

// ADC Interrupt Handler
void ADCIntHandler(){
  // Clear Interrupt
  ADCIntClear(ADC0_BASE, 0);
  
  // Update value
  ADCSequenceDataGet(ADC_BASE, 3, &adc_last_value);
  
  // Call Periodic function
  function(adc_last_value);
    
  // Check to see if we're done
  if (--adc_samples == 0){
    // Disable ADC Interrupts
    ADCIntDisable(ADC0_BASE, 0);
    
    // Update status
    adc_status = ADC_IDLE;
  }
}

// Initialize ADC w/ Timer0
void ADC_Init(unsigned long freq){
    // Enable Peripheral 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC);
    IntEnable(INT_ADC0);
    
    // ADC Pins are dedicated.
    // No need to get GPIO Type
    
    // Set status
    adc_status = ADC_IDLE;
}

// Get status of ADC
unsigned short ADC_Status(){  return adc_status; }

// Internally start ADC
void ADC_Enable(unsigned int samples){
    // Set number of samples before finished
    adc_samples = samples;
    
    // Change status
    adc_status = ADC_BUSY;
    
    // Enable ADC Interrupts
    //ADCIntEnable(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC0_BASE, 3);
}

// Perform ADC on Channel
unsigned short ADC_Read(unsigned int channelNum){
    // Make sure channelNum is value 0 to 3
    ASSERT(channelNum < 4);
    
    // Setup ADC Sequence
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, channelNum, channelNum | ADC_CTL_IE | ADC_CTL_END);
    
    // Start Conversion
    ADC_Enable(1);
    
    // Wait for ADC to finish
    while (adc_status != ADC_IDLE);
    
    // Return value
    return (short) adc_last_value;
}

// Collect multiple samples from single ADC Channel
void ADC_Collect(unsigned int channelNum, unsigned int freq, void (*func)(unsigned short), unsigned int samples){
  unsigned int i;
  
  // Check parameters
  ASSERT(channelNum < 4);
  ASSERT(freq >= 100);
  ASSERT(freq <= 10000);
  
  // Set channel to sample from
  ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  ADCSequenceStepConfigure(ADC0_BASE, 0, channelNum, channelNum | ADC_CTL_IE | ADC_CTL_END);
  
  // Set Sample Frequency
  TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / freq);
  
  // Set function pointer
  function = func;
  
  // Start conversions
  ADC_Enable(samples);
  
  // Collect samples as they become availble
  adc_last_value = ADC_SAMPLE_NOT_READY;
  for(i=0; i<samples; i++){
    // Idle until sample is ready
    while(adc_last_value == ADC_SAMPLE_NOT_READY);
    
    // Read new sample
    //buffer[i] = (unsigned short) adc_last_value;
    
    // Reset adc_last_sample
    adc_last_value = ADC_SAMPLE_NOT_READY;
    
    /* Should not need this
    if (adc_status == ADC_IDLE)
      break;
    */
  }
  
  // Set status to finished
  adc_status = ADC_IDLE;
}
