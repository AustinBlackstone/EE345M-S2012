/**
Filename:       debug.c
Name:           Cruz Monrreal II, Austin Blackstone
Creation Date:  01/25/2012
Lab #:          1
TA:             Zahidul
Last Revision:  10/30/2012
Description:    debugging functions for RTOS
*/

#include "debug.h"
#include "uart.h"


long MaxJitter;
long MinJitter;
unsigned long const JitterSize;
unsigned long JitterHistogram[];


void Jitter(void){;}   // TODO: prints jitter information (write this)

void Interpreter(void){ // Pipe to UART Parser (localvars in uart.c, etc...)
  while(1){
    UARTParse();
  }
}
