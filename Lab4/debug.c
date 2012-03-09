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
#include "os.h"

#define JITTERSIZE1 600
#define JITTERSIZE  64
#define PERIOD		TIME_1MS/2 

extern void oLED_Message(int,int,char *,long);

long MaxJitter;
long MinJitter;
long jitter;
long LastTime;
int jindex;

unsigned long const JitterSize;
unsigned long JitterHistogram[JITTERSIZE]={0,};


unsigned long const JitterSize = JITTERSIZE;

void Jitter(void){
	oLED_Message(1,3,"Jitter(us) =", MaxJitter-MinJitter);
}

long JitterStart(void){ 
	long thisTime;
	thisTime=OS_Time();
	return thisTime;
}

void JitterFinish(long thisTime){
	if(LastTime!=0){
	  jitter = OS_TimeDifference(thisTime,LastTime)/50-PERIOD/50;  // in usec
      if(jitter > MaxJitter){
        MaxJitter = jitter;
      }
      if(jitter < MinJitter){
        MinJitter = jitter;
      }        // jitter should be 0
      jindex = jitter+JITTERSIZE/2;   // us units
      if(jindex<0)jindex = 0;
      if(jindex>=JitterSize) jindex = JITTERSIZE-1;
      JitterHistogram[jindex]++; 
    }
    LastTime = thisTime;	

}
void JitterInit(void){
	LastTime=0;
}

void Interpreter(void){ // Pipe to UART Parser (localvars in uart.c, etc...)
  while(1){
    UARTParse();
  }
}
