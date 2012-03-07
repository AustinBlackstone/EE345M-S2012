/**
Filename:       debug.c
Name:           Cruz Monrreal II, Austin Blackstone
Creation Date:  01/25/2012
Lab #:          1
TA:             Zahidul
Last Revision:  10/30/2012
Description:    debugging functions for RTOS
*/

#ifndef __DEBUG_H
#define __DEBUG_H 1

void Jitter(void);   	// prints jitter information
void JitterInit(void);  //sets LastTime=0
long JitterStart(void);	//gets current time (renamed OS_Time function)
void JitterFinish(long);//measures the jitter, taken from lab2.c
void Interpreter(void); // Pipe to UART Parser (localvars in uart.c, etc...)

#endif
