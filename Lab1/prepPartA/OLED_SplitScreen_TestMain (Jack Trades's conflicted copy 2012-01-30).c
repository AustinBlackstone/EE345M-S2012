// OLEDTestMain.c
// Runs on LM3S8962
// Test OutputD.c by sending various characters and strings to
// the OLED display and verifying that the output is correct.
// Daniel Valvano
// July 28, 2011

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
   Section 3.4.5

 Copyright 2011 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdio.h>
//#include "OLED_SplitScreen.h"
#include "Output.h"
#include "Output.c"
#include "rit128x96x4.h"

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
__asm void
Delay(unsigned long ulCount)
{
    subs    r0, #1
    bne     Delay
    bx      lr
}
int main(void){
  int i;
  int j;
  Output_Init();
  Output_Color(15);
  for(j=0,i=0;;j++,i++){
  	oLED_Message(top,j,"Hello Top ",i);
  	oLED_Message(bottom,j,"Hello Bottom ", i);
  //	Delay(4000000);
  	}
  while(1){};
  
}
