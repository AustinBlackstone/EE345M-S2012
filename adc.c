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

static void (*function)(unsigned short);
unsigned long adc_last_value;

// ADC Interrupt Handler
void ADCIntHandler(){
    // Update value
    ADCSequenceDataGet(ADC0_BASE, 3, &adc_last_value);
    
    // Clear Interrupt
    ADCIntClear(ADC0_BASE, 3);
    
    // Call Periodic function
    if (function != 0)
        function(adc_last_value);
}

// Initialize ADC w/ Timer0
void ADC_Init(){
	// See OS_Init
    
    // Set status
    //adc_status = ADC_IDLE;
    function = 0;
}

// Perform ADC on Channel
unsigned short ADC_Read(unsigned int channelNum){
    // Make sure channelNum is value 0 to 3
    ASSERT(channelNum < 4);
    
    // Disable calling function
    function = 0;
    
    // Turn off interrupts
    IntDisable(INT_TIMER0B);
	IntDisable(INT_ADC3);
    
    // Setup ADC Sequence
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, channelNum | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntEnable(ADC0_BASE, 3);
	IntEnable(INT_ADC3);
    
    // Start Conversion
    ADCProcessorTrigger(ADC0_BASE, 3);
    
    // Wait for ADC to finish
    while (!ADCIntStatus(ADC0_BASE, 3, false));
    
    // Return value
    return (short) adc_last_value;
}

// Collect multiple samples from single ADC Channel
void ADC_Collect(unsigned int channelNum, unsigned int freq, void (*func)(unsigned short)){
    // Check parameters
    ASSERT(channelNum < 4);
    ASSERT(freq >= 100);
    ASSERT(freq <= 10000);
    
    // Turn off interrupts
    IntDisable(INT_TIMER0B);
	IntDisable(INT_ADC3);
    
    // Set function pointer
    function = func;
    
    // Setup ADC Sequence
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, channelNum | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntEnable(ADC0_BASE, 3);
	IntEnable(INT_ADC3);
    
    // Setup hardware timer  
    TimerLoadSet(TIMER0_BASE, TIMER_B, SysCtlClockGet() / freq);
    TimerControlTrigger(TIMER0_BASE, TIMER_B, true);
    TimerEnable(TIMER0_BASE, TIMER_B);
    IntEnable(INT_TIMER0B);
    
    // Collect Data
    /*for(i=0; i<samples; i++){
        // Wait for ADC to finish
        while (adc_last_value == ADC_SAMPLE_NOT_READY);
        
        // Reset control variable
        adc_last_value = ADC_SAMPLE_NOT_READY;
    }*/
}
