/**
Filename:       uart_echo_mod.c
Name:           Austin Blackstone, Cruz Monrreal II
Creation Date:  02/01/2012
Lab #:          1
TA:             Zahidul
Last Revision:  02/04/2012
Description:    use to receive and echo commands, will eventually be turned into interpreter
				takes in commands from putty on UART0 and prints out to the screen on line0
*/

/*
NOTE: this project is based off of and uses code from TI's uart_echo project
		we just added software fifo's betweeen the hardware fifos and main program
		in no way shape or form do we claim credit for UARTIntHandler, UartSend, UartGet,
		or any other functions in the package.
*/


#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "../../rit128x96x4.h"
#include "FIFO.h"
#include "os.h"

	//
	//Add FIFO's for Input[Rx] and Output[Tx]
	//returns 1 on sucess, 0 on failure
	//
	AddIndexFifo(UARTIn,128,char,1,0)
	AddIndexFifo(UARTOut,128,char,1,0)
  	AddIndexFifo(UARTCmd,64,char,1,0)
  	char outChar;
	int check;
	int i;


//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************

void
UARTIntHandler(void)
{
	
    unsigned long ulStatus;
	
	
    //
    // Get the interrrupt status.
    //
    ulStatus = UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(UART0_BASE, ulStatus);

    //
    // Loop while there are characters in the receive FIFO.
    //
	while(UARTCharsAvail(UART0_BASE))
    {
        //
        // Read the next character from the UART and write it back to the UART.
        //

    		//take input from hardware in fifo and put into harware out fifo	
    		UARTInFifo_Put(UARTCharGetNonBlocking(UART0_BASE));        
        
        //UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE));
  	    //oLED_Message(bottom,0,*pucBuffer,0);
    }

	//
 	// while space available and there is something to output, output to hardware fifo
	//
	while(UARTSpaceAvail(UART0_BASE) && UARTOutFifo_Get(&outChar) ){ //possible ERROR on &outChar, not sure if i used it correctly
		UARTCharPutNonBlocking(UART0_BASE, outChar);
		oLED_Message(bottom,0,&outChar,-0);
			
	}	
	

}

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void
UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        //
        // Write the next character to the UART.
        //
        //UARTCharPutNonBlocking(UART0_BASE, *pucBuffer++);
        UARTOutFifo_Put(*pucBuffer++);
    }
}


void dummy(void){}


void UARTInit(void){
	UARTInFifo_Init();		  //Init FIFO's for UART
	UARTOutFifo_Init();
	UARTCmdFifo_Init();
    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	
	//
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
  
    //
    // Enable the UART interrupt.
    //
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    //
    // Prompt for text to be entered.
    //
    //UARTSend((unsigned char *)"$ ", 12);
	  UARTOutFifo_Put('$');		//command line character
	  UARTOutFifo_Put(' ');
	  UARTCharPutNonBlocking(UART0_BASE, '$');

}


//*****************************************************************************
//
// This example demonstrates how to send a string of data to the UART.
//
//*****************************************************************************
int LAB1main()
{
	  char test;
    char str[32];
    char i, tmp;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);
	  UARTInFifo_Init();
	  UARTOutFifo_Init();
	  UARTCmdFifo_Init();

    //
    // Initialize the OLED display and write status.
    //
    RIT128x96x4Init(1000000);
    //RIT128x96x4StringDraw("UART Echo",            36,  0, 15);
    //RIT128x96x4StringDraw("Port:   Uart 0",       12, 16, 15);
    //RIT128x96x4StringDraw("Baud:   115,200 bps",  12, 24, 15);
    //RIT128x96x4StringDraw("Data:   8 Bit",        12, 32, 15);
    //RIT128x96x4StringDraw("Parity: None",         12, 40, 15);
    //RIT128x96x4StringDraw("Stop:   1 Bit",        12, 48, 15);
	//oLED_Message(top,0,"$ ",-0);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    // Test
    OS_AddPeriodicThread(dummy, 10000, 1);
    
    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
  
    //
    // Enable the UART interrupt.
    //
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    //
    // Prompt for text to be entered.
    //
    //UARTSend((unsigned char *)"$ ", 12);
	  UARTOutFifo_Put('$');		//command line character
	  UARTOutFifo_Put(' ');
	  UARTCharPutNonBlocking(UART0_BASE, '$');
    //
    // Loop forever echoing data through the UART.
    //
    while(1)
    {		
    		while(UARTInFifo_Get(&test)){
            // Echo to terminal
            UARTOutFifo_Put(test);
            
            // Parse input
            switch(test){
                case '\r':
                    i=0;
                    
                    // Generate String
                    while(UARTCmdFifo_Get(&tmp))
                        str[i++]=tmp;
                        
                    str[i]='\n'; // Terminalte String
                    
                      // If/Else Blocks for parsing commands
                    if (strstr(str, "echo")){
                      // Remove 'echo' from command
                      strncpy(str, str+5, strlen(str)-4);
                        
                      // Echo string to oLED
                      oLED_Message(bottom, 0, str, -0);
                    
                      // New Line
                      UARTOutFifo_Put('\r');
                      UARTOutFifo_Put('\n');
                    
                      i=0;
                      while((tmp=str[i++]) != '\n')
                        UARTOutFifo_Put(tmp);
                    }
                    
                    
                    // New Line
                    UARTOutFifo_Put('\r');
                    UARTOutFifo_Put('\n');
                    
                    // Reset String
                    memset(str, 0, 32);
                        
                    // Prompt user for more input
                	  UARTOutFifo_Put('$');
                 	  UARTOutFifo_Put(' ');
                    
                    break;
                default:
                    UARTCmdFifo_Put(test);
            }
            
    		UARTIntHandler();
		}
	}
    //return 0;
}
