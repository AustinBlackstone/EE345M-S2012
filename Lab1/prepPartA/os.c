#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include "lm3s1968.h"
#include "driverlib/timer.h"
#include "os.h"

unsigned long os_counter;
void(*func)(void) = 0;

// Timer2 Interrupt Handler
void Timer2A_Handler()
{
    // Ack Timer2
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
    // Inc OS Counter
    os_counter++;
    
    // Run Timer Task
    func();
}

int OS_AddPeriodicThread(void(*task)(void) , unsigned long period, unsigned long priority){
    // Save 
    func = task;
    
    // Reconfigure Timer2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER2_BASE, TIMER_CFG_32_BIT_PER);
    TimerControlTrigger(TIMER2_BASE, TIMER_A, true);
    TimerLoadSet(TIMER2_BASE, TIMER_A, period);
    TimerEnable(TIMER2_BASE, TIMER_A);
    
    // Priority?
    
    return 1;
} 

void OS_ClearMsTime(void){
    // Reset timer counter
    os_counter = 0;
    
    // Disable hardware timer
    TimerDisable(TIMER2_BASE, TIMER_A);
}

unsigned long OS_MsTime(void){ return os_counter; }

