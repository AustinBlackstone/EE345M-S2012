#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"

#include "FIFO.h"
#include "uart.h"
#include "rit128x96x4.h"

AddIndexFifo(In, 128,char,1,0);
AddIndexFifo(Out,128,char,1,0);
AddIndexFifo(Cmd, 64,char,1,0);
char uart_outChar;

void UARTIntHandler(void)
{
	
  unsigned long ulStatus;

  // Get the interrrupt status.
  ulStatus = UARTIntStatus(UART0_BASE, true);

  // Clear the asserted interrupts.
  UARTIntClear(UART0_BASE, ulStatus);

  // Loop while there are characters in the receive FIFO.
  while(UARTCharsAvail(UART0_BASE))
    {
      // Read the next character from the UART and write it back to the UART.
  
  		//take input from hardware in fifo and put into harware out fifo	
  		InFifo_Put(UARTCharGetNonBlocking(UART0_BASE));        
      
      //UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE));
      //oLED_Message(bottom,0,*pucBuffer,0);
    }

 	// while space available and there is something to output, output to hardware fifo
	while(UARTSpaceAvail(UART0_BASE) && OutFifo_Get(&uart_outChar) ){ //possible ERROR on &outChar, not sure if i used it correctly
		UARTCharPutNonBlocking(UART0_BASE, uart_outChar);
//		oLED_Message(bottom,0,&outChar,-0);
			
	}
}

void UARTParse(){
  char tmp, test;
  char *str;
  int i;
  
  while(InFifo_Get(&test)){
      // Echo to terminal
      OutFifo_Put(test);
      
      // Parse input
      switch(test){
          case '\r':
              i=0;
              
              // Generate String
              while(CmdFifo_Get(&tmp))
                  str[i++]=tmp;
                  
              str[i]='\n'; // Terminalte String
              
                // If/Else Blocks for parsing commands
              if (strstr(str, "echo")){ 
                // Remove 'echo' from command
                strncpy(str, str+5, strlen(str)-4);
                  
                // Echo string to oLED
                oLED_Message(bottom, 0, str, -0);
              
                // New Line
                OutFifo_Put('\r');
                OutFifo_Put('\n');
              
                i=0;
                while((tmp=str[i++]) != '\n')
                  OutFifo_Put(tmp);
              }
              
              
              // New Line
              OutFifo_Put('\r');
              OutFifo_Put('\n');
              
              // Reset String
              memset(str, 0, 32);
                  
              // Prompt user for more input
          	  OutFifo_Put('$');
           	  OutFifo_Put(' ');
              
              break;
          default:
              CmdFifo_Put(test);
      }
    }
      
    UARTIntHandler();
}


void UARTInit(){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);  

  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));


  
  IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

  OutFifo_Put('$');		//command line character
  OutFifo_Put(' ');
  UARTCharPutNonBlocking(UART0_BASE, '$');

  InFifo_Init();
  OutFifo_Init();
  CmdFifo_Init();
//  UARTCharPutNonBlocking(UART0_BASE, '$');
}
