#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "lm3s1968.h"
#include "driverlib/timer.h"
#include "os.h"

unsigned long os_counter;
void(*func)(void);

// Timer2 Interrupt Handler
void Timer2A_Handler()
{
    // IO Off
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
    
    // Ack Timer1
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    
    // Inc OS Counter
    os_counter++;
    
    // Run Timer Task
    func();
    
    // IO On
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 1);
    
}

int OS_AddPeriodicThread(void (*task)(void) , unsigned long period, unsigned long priority){
    // Save 
    func = task;
    
    // Reconfigure Timer1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    TimerConfigure(TIMER2_BASE, TIMER_CFG_A_PERIODIC);
    //TimerControlTrigger(TIMER2_BASE, TIMER_A, true);
    TimerLoadSet(TIMER2_BASE, TIMER_A, period);
    TimerEnable(TIMER2_BASE, TIMER_A);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER2A);
    
    // Priority?
    
    // Debugging IO
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 1);
    
    return 1;
} 

void OS_ClearMsTime(void){
    // Reset timer counter
    os_counter = 0;
    
    // Disable hardware timer
    TimerDisable(TIMER2_BASE, TIMER_A);
}

unsigned long OS_MsTime(void){ return os_counter; }

