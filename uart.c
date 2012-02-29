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

AddIndexFifo(UARTIn, 128,char,1,0);
AddIndexFifo(UARTOut,128,char,1,0);
AddIndexFifo(UARTCmd, 64,char,1,0);
//char uart_outChar;

void UARTIntHandler(void)
{
    static char tmp;
    // Clear the asserted interrupts.
    UARTIntClear(UART0_BASE, UARTIntStatus(UART0_BASE, true));
    
    // Read Hardware
    while (UARTCharsAvail(UART0_BASE)){
        tmp = UARTCharGetNonBlocking(UART0_BASE);
        UARTInFifo_Put(tmp);
        
        // Echo back to terminal
        UARTOutFifo_Put(tmp);
    }
    
    while (UARTSpaceAvail(UART0_BASE) && UARTOutFifo_Get(&tmp) != 0){
        UARTCharPutNonBlocking(UART0_BASE, tmp);
        
        if (tmp == '\r')
            UARTCharPutNonBlocking(UART0_BASE, '\n');
    }
}

void UARTParse(){
    char tmp, test;
    char str[65];
    int i;
  
    while(UARTInFifo_Get(&test)){
        // Parse input
        switch(test){
            case '\r':
                i=0;
                
                // Generate String
                while(UARTCmdFifo_Get(&tmp) && i<64)
                    str[i++]=tmp;
                
                str[i]='\n'; // Terminate String
                
                // If/Else Blocks for parsing commands
                if (strstr(str, "echo")){ 
                    // Remove 'echo' from command
                    strncpy(str, str+5, strlen(str)-4);
                    
                    // Echo string to oLED
                    //oLED_Message(bottom, 0, str, -0);
                    
                    i=0;
                    while((tmp=str[i++]) != '\n')
                        UARTOutFifo_Put(tmp);
                }
                
                // New Line
                UARTOutFifo_Put('\r');
                
                // Reset String
                memset(str, 0, 32);
                
                // Prompt user for more input
                UARTOutFifo_Put('$');
                UARTOutFifo_Put(' ');
                
                UARTIntHandler();
                break;
            default:
                UARTCmdFifo_Put(test);
        }
    }
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
    
    UARTInFifo_Init();
    UARTOutFifo_Init();
    UARTCmdFifo_Init();
    
    UARTCharPutNonBlocking(UART0_BASE, '$');
    UARTCharPutNonBlocking(UART0_BASE, ' ');
}
