#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"

#include "driverlib/uart.h"
#include "driverlib/gpio.h"

void UARTIntHandler(void)
{
    unsigned long status;

    // Get interrupt status
    status = UARTIntStatus(UART0_BASE, true);

    // Ack interrupts
    UARTIntClear(UART0_BASE, status);

    // Loop while there are characters in the receive FIFO
    while(UARTCharsAvail(UART0_BASE)){
        // Echo input to terminal
        UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE));
        
        // Put into Received FIFO
        
    }
}

void UART_Init(){
    // Enable Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Set Pin Types
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART for 115,200, 8-N-1 operation.
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);

    // Enable the UART interrupt.
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

void UART_Send(unsigned char* str, unsigned long length){
    // Loop through all characters
    while(length--){
        // Write character into UART Buffer
        UARTCharPutNonBlocking(UART0_BASE, *str++);
    }
}
