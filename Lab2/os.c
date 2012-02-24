// filename **********OS.c***********
// Real Time Operating System for Labs 2 and 3 
// Austin Blackstone, Cruz Monnreal II 2/16/2012
//***********Ready to go*************
// You may use, edit, run or distribute this file 
// You are free to change the syntax/organization of this file
 
#include "os.h"
#include "lm3s8962.h"
#include "Startup.h"
#include "uart_echo_mod.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"

#include "rit128x96x4.h"
#include "FIFO.h"
#include "adc.h"
#include "uart.h"
#define  NULL 0

tcbType tcbs[NUMTHREADS]; // allocated space for all TCB's to be used in this program
tcbType *RUNPT;
tcbType *NEXTRUNPT;

//globals 
int IDCOUNT;	//incrimented to generate unique thread id's
long int OSMAILBOX; // contains mailbox data for OSMailBox
void (*BUTTONTASK)(void);   // pointer to task that gets called when you press a button 
long SDEBOUNCEPREV;  // used for debouncing 'select' switch, contains previous value
long TIMELORD; 
extern unsigned long NumCreated; // number of foreground threads created, referenced from lab2.c
	
void(*periodicFunc)(void);
  
//Semaphores used for program
Sema4Type oled_free;
Sema4Type OSMailBoxSema4;

//Fifos
#define	OSFIFOSIZE	64
AddIndexFifo(OS , OSFIFOSIZE ,long int, 1, 0 )


// TImer0A Int Handler
void Timer0A_Handler(){
  // Clear Interrupt
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  
  // Run Periodic function
  periodicFunc();
}

// ******** OS_Init ************
// initialize operating system, disable interrupts until OS_Launch
// initialize OS controlled I/O: serial, ADC, systick, select switch and timer2 
// input:  none
// output: none
void OS_Init(void){
	DisableInterrupts();
	RUNPT=0;

// Enable processor interrupts.
    //
    //IntMasterEnable();
	TIMELORD=0; //initialize the system counter for use with thingies (no shit)

	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);	//Init System Clock

	//Systick Init (Thread Scheduler)
		 //taken care of in OS_Launch
     
  // Init ADC Timer  (Default @ 1KHz)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
  TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  IntEnable(INT_TIMER0A);
  
  // Init Debugging LED
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);

	//Semaphores, OS Stuff
	//OS_InitSemaphore(&oled_free,0);
	//OS_InitSemaphore(&OSMailBoxSema4,0);
	//OS_MailBox_Init();
	
	//UART & OLED 
//	RIT128x96x4Init(1000000); //Init OLED
//	UARTInit();
//	RIT128x96x4StringDraw("Hello World", 0, 12, 15);
	
	
	//ADC
//  ADC_Init(1000); // Init ADC to run @ 1KHz


	//Select Switch (button press) Init	(select switch is PF1) (pulled from page 67 of the book and modified for PF1...i think)
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  /*GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
  IntEnable(INT_GPIOF);  
  //IntPrioritySet(INT_GPIOF, 0x00);
  GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_BOTH_EDGES);
  GPIOPinIntEnable(GPIO_PORTF_BASE, GPIO_PIN_1);*/
  //NVIC_EN0_R |= 0x40000000;     // (h) enable interrupt 2 in NVIC (Not sure what Stellarisware function replaces this)
  
  
  /* This works for now, but Stellarisware Function owuld be nice */
  SYSCTL_RCGC2_R |= 0x00000020; // (a) activate port F
//	delay = SYSCTL_RCGC2_R;		    //delay, cause i said so
	GPIO_PORTF_DIR_R &= ~0x02;    // (c) make PF1 in
	GPIO_PORTF_DEN_R |= 0x02;     //     enable digital I/O on PF1
	GPIO_PORTF_IS_R &= ~0x02;     // (d) PF1 is edge-sensitive
	GPIO_PORTF_IBE_R &= ~0x02;    //     PF1 is not both edges
	GPIO_PORTF_IEV_R &= ~0x02;    //     PF1 falling edge event
	GPIO_PORTF_ICR_R = 0x02;      // (e) clear flag4
	GPIO_PORTF_IM_R |= 0x02;      // (f) arm interrupt on PF1
	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|(0<<21); // (g) priority (shifted into place) (will get set in OS_AddButtonTask)
	NVIC_EN0_R |= 0x40000000;     // (h) enable interrupt 2 in NVIC
	//dont enable interrupts 
	GPIO_PORTF_PUR_R |= 0x02;	    //add pull up resistor, just for shits and giggles
  
/*	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	  	// Enable GPIOF
  	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);	// make Pin 1 an Input
  	GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);			//
*/

//TODO: i have no fucking clue what to do here...

return;
}

// ******** OS_InitSemaphore ************
// initialize semaphore 
// input:  pointer to a semaphore
// output: none
void OS_InitSemaphore(Sema4Type *semaPt, long initialValue){
	semaPt->Value=initialValue;   //assuming all semaphores are initialized to 1
} 

// ******** OS_Wait ************
// decrement semaphore and spin/block if less than zero
// input:  pointer to a counting semaphore
// output: none
void OS_Wait(Sema4Type *s){
	DisableInterrupts();
	while( s->Value <= 0){
		//TODO: implement Blocking
		EnableInterrupts();
		DisableInterrupts();
	}
	s->Value--;
	EnableInterrupts();
}

// ******** OS_Signal ************
// increment semaphore, wakeup blocked thread if appropriate 
// input:  pointer to a counting semaphore
// output: none
void OS_Signal(Sema4Type *s){
	long status;
	status=StartCritical();
  s->Value++;
	EndCritical(status);
	//TODO: wakeup blocked thread
}

// ******** OS_bWait ************
// if the semaphore is 0 then spin/block
// if the semaphore is 1, then clear semaphore to 0
// input:  pointer to a binary semaphore
// output: none
void OS_bWait(Sema4Type *semaPt){
	DisableInterrupts();
	while( semaPt->Value == 0){
		//TODO: implement Blocking
		EnableInterrupts();
		DisableInterrupts();
	}
	semaPt->Value =0;
	EnableInterrupts();

return;
}

// ******** OS_bSignal ************
// set semaphore to 1, wakeup blocked thread if appropriate 
// input:  pointer to a binary semaphore
// output: none
void OS_bSignal(Sema4Type *semaPt){
	long status;
	status=StartCritical();
	semaPt->Value= 1;
	EndCritical(status);

	//TODO: wakeup blocked thread

return;
}

//******** OS_AddThread *************** 
// add a foregound thread to the scheduler
// Inputs: pointer to a void/void foreground task
//         number of bytes allocated for its stack
//         priority (0 is highest)
// Outputs: 1 if successful, 0 if this thread can not be added
// stack size must be divisable by 8 (aligned to double word boundary)
// In Lab 2, you can ignore both the stackSize and priority fields
// In Lab 3, you can ignore the stackSize fields
int OS_AddThread(void(*task)(void), 
   unsigned long stackSize, unsigned long priority){
	  int x=0, found, status;
    /*
    // Find unused thread
    while(x<NUMTHREADS)
      if (tcbs[x++].used == 0)
        break;
        
    // No unused threads found
    if (x=NUMTHREADS)
      return 0;

    // Unused thread found. x+1=index of thread
    x--;
    
    if ()*/
    
	  for(x=0,found=0;(x<NUMTHREADS) && (found==0);x++){	 //loop untill you find a non used thread
			if(tcbs[x].used==0){
				found=1;
				status=StartCritical();			//possible critical section?	
				OS_ThreadInit(&tcbs[x],0xFFFA-x);
				tcbs[x].used=1;
				if(NumCreated==0){					//First Thread Ever, needs special case for it's RUNPT, b/c there is no run pointer previously
					RUNPT=&tcbs[x];					//set run pointer to point to initial thread
					tcbs[x].next=RUNPT;				//it is only thread, so it points to itself for prev and next
					tcbs[x].prev=RUNPT;
				} else{
					tcbs[x].next=RUNPT;				// set prev / next on new thread
					tcbs[x].prev=RUNPT->prev;		//
					RUNPT->prev->next=&tcbs[x];		// insert thread into current linked list of running threads
					RUNPT->prev=&tcbs[x];			//
				
				}
				tcbs[x].stack[STACKSIZE-2]=(long)task; //POSSIBLE ERROR, PC=task, i think this works... right??
				EndCritical(status);				//end of possible critical section?
				tcbs[x].realPriority=priority;
				tcbs[x].workingPriority=priority;
				tcbs[x].id=++IDCOUNT;
				
				//TODO: implement adjustable stacksize using input variable 'stackSize'
				
			}	  
	  }
return found;
}

//******** OS_Id *************** 
// returns the thread ID for the currently running thread
// Inputs: none
// Outputs: Thread ID, number greater than zero 
unsigned long OS_Id(void){
	return RUNPT->id;
}

//******** OS_AddPeriodicThread *************** 
// add a background periodic task
// typically this function receives the highest priority
// Inputs: pointer to a void/void background function
//         period given in system time units
//         priority 0 is highest, 5 is lowest
// Outputs: 1 if successful, 0 if this thread can not be added
// It is assumed that the user task will run to completion and return
// This task can not spin, block, loop, sleep, or kill
// This task can call OS_Signal  OS_bSignal	 OS_AddThread
// You are free to select the time resolution for this function
// This task does not have a Thread ID
// In lab 2, this command will be called 0 or 1 times
// In lab 2, the priority field can be ignored
// In lab 3, this command will be called 0 1 or 2 times
// In lab 3, there will be up to four background threads, and this priority field 
//           determines the relative priority of these four threads
int OS_AddPeriodicThread(void(*task)(void), 
  unsigned long period, unsigned long priority){
  
  // Set timer period
  TimerLoadSet(TIMER0_BASE, TIMER_A, period);
  TimerEnable(TIMER0_BASE, TIMER_A);
  IntEnable(INT_TIMER0A);
  
  // Set function pointer
  periodicFunc = task;
  
  return 1; // For now
}

//******** OS_AddButtonTask *************** 
// add a background task to run whenever the Select button is pushed
// Inputs: pointer to a void/void background function
//         priority 0 is highest, 5 is lowest
// Outputs: 1 if successful, 0 if this thread can not be added
// It is assumed that the user task will run to completion and return
// This task can not spin, block, loop, sleep, or kill
// This task can call OS_Signal  OS_bSignal	 OS_AddThread
// This task does not have a Thread ID
// In labs 2 and 3, this command will be called 0 or 1 times
// In lab 2, the priority field can be ignored
// In lab 3, there will be up to four background threads, and this priority field 
//           determines the relative priority of these four threads
int OS_AddButtonTask(void(*task)(void), unsigned long priority){
	//TODO: add this back in to implement priority
//	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|(priority<<21); // (g) priority (shifted into place)
	BUTTONTASK=task;   //POSSIBLE ERROR
	//for the record the header on this is confusing as balls, this doesnt add a thread, it sets up an interrupt so IT will call a function that will add a thread. FML

return 1;
}

//******** OS_AddDownTask *************** 
// add a background task to run whenever the Down arrow button is pushed
// Inputs: pointer to a void/void background function
//         priority 0 is highest, 5 is lowest
// Outputs: 1 if successful, 0 if this thread can not be added
// It is assumed user task will run to completion and return
// This task can not spin block loop sleep or kill
// It can call issue OS_Signal, it can call OS_AddThread
// This task does not have a Thread ID
// In lab 2, this function can be ignored
// In lab 3, this command will be called will be called 0 or 1 times
// In lab 3, there will be up to four background threads, and this priority field 
//           determines the relative priority of these four threads
int OS_AddDownTask(void(*task)(void), unsigned long priority){
return 0;
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// You are free to select the time resolution for this function
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(unsigned long sleepTime){
	RUNPT->sleep = sleepTime;
	//cause SYSTICK Interrupt / switch threads
	NVIC_ST_CURRENT_R =0;
	NVIC_INT_CTRL_R = 0x04000000;

return;
}

// ******** OS_Kill ************
// kill the currently running thread, release its TCB memory
// input:  none
// output: none
void OS_Kill(void){
	//set used=0, take it out of linked list, Possible ERROR, not 100% sure of efficacy
	DisableInterrupts();
	RUNPT->used=0;
	RUNPT->prev->next=RUNPT->next;
	RUNPT->next->prev=RUNPT->prev;
	RUNPT->id=0x0DEADDEAD; //get it, the threads dead, hehehe, clever little bastard aint i?
	//trigger SysTick, .'. the thread scheduler to run, next loop around this thread wont be here
	NVIC_ST_CURRENT_R =0;
	NVIC_INT_CTRL_R = 0x04000000;
	EnableInterrupts();
return;
}

// ******** OS_Suspend ************
// suspend execution of currently running thread
// scheduler will choose another thread to execute
// Can be used to implement cooperative multitasking 
// Same function as OS_Sleep(0)
// input:  none
// output: none
void OS_Suspend(void){
	//trigger SysTick (aka the thread scheduler)
	NVIC_ST_CURRENT_R =0;
	NVIC_INT_CTRL_R = 0x04000000;
return;
}
 
// ******** OS_Fifo_Init ************
// Initialize the Fifo to be empty
// Inputs: size
// Outputs: none 
// In Lab 2, you can ignore the size field
// In Lab 3, you should implement the user-defined fifo size
// In Lab 3, you can put whatever restrictions you want on size
//    e.g., 4 to 64 elements
//    e.g., must be a power of 2,4,8,16,32,64,128
void OS_Fifo_Init(unsigned long size){
return;
}

// ******** OS_Fifo_Put ************
// Enter one data sample into the Fifo
// Called from the background, so no waiting 
// Inputs:  data
// Outputs: true if data is properly saved,
//          false if data not saved, because it was full
// Since this is called by interrupt handlers 
//  this function can not disable or enable interrupts
int OS_Fifo_Put(unsigned long data){
return OSFifo_Put(data);
}

// ******** OS_Fifo_Get ************
// Remove one data sample from the Fifo
// Called in foreground, will spin/block if empty
// Inputs:  none
// Outputs: data 
unsigned long OS_Fifo_Get(void){
long temp;
return OSFifo_Get(&temp);
}

// ******** OS_Fifo_Size ************
// Check the status of the Fifo
// Inputs: none
// Outputs: returns the number of elements in the Fifo
//          greater than zero if a call to OS_Fifo_Get will return right away
//          zero or less than zero if the Fifo is empty 
//          zero or less than zero  if a call to OS_Fifo_Get will spin or block
long OS_Fifo_Size(void){
return OSFifo_Size();
}

// ******** OS_MailBox_Init ************
// Initialize communication channel
// Inputs:  none
// Outputs: none
void OS_MailBox_Init(void){
	OS_InitSemaphore(&OSMailBoxSema4,0);
return;
}

// ******** OS_MailBox_Send ************
// enter mail into the MailBox
// Inputs:  data to be sent
// Outputs: none
// This function will be called from a foreground thread
// It will spin/block if the MailBox contains data not yet received 
void OS_MailBox_Send(unsigned long data){
	OS_Wait(&OSMailBoxSema4);
	OSMAILBOX=data;	
return;
}

// ******** OS_MailBox_Recv ************
// remove mail from the MailBox
// Inputs:  none
// Outputs: data received
// This function will be called from a foreground thread
// It will spin/block if the MailBox is empty 
unsigned long OS_MailBox_Recv(void){
	unsigned long temp;
	while(OSMailBoxSema4.Value >0 ){  //POSSIBLE ERROR in how i acess value, while loop should be removed or rethought eventually
	//implements spinning when mailbox is empty
	;//TODO: implement blocking
	}
	temp = OSMAILBOX;
	OS_Signal(&OSMailBoxSema4);
return temp;
}

// ******** OS_Time ************
// reads a timer value 
// Inputs:  none
// Outputs: time in 20ns units, 0 to max
// The time resolution should be at least 1us, and the precision at least 12 bits
// It is ok to change the resolution and precision of this function as long as 
//   this function and OS_TimeDifference have the same resolution and precision 
unsigned long OS_Time(void){
return SysTickValueGet(); //ERROR, this is TOTALLY INCORRECT but it approximates it, so bite me.
}

// ******** OS_TimeDifference ************
// Calculates difference between two times
// Inputs:  two times measured with OS_Time
// Outputs: time difference in 20ns units 
// The time resolution should be at least 1us, and the precision at least 12 bits
// It is ok to change the resolution and precision of this function as long as 
//   this function and OS_Time have the same resolution and precision 
unsigned long OS_TimeDifference(unsigned long start, unsigned long stop){
return (long)(start-stop); //start should be higher, stop should be lower, since the SysTick counter counts down
}

// ******** OS_ClearMsTime ************
// sets the system time to zero from Lab 1)
// Inputs:  none
// Outputs: none
// You are free to change how this works
void OS_ClearMsTime(void){
	int status;
	status=StartCritical();
	TIMELORD=0;
	EndCritical(status);
return;
}

// ******** OS_MsTime ************
// reads the current time in msec (from Lab 1)
// Inputs:  none
// Outputs: time in ms units
// You are free to select the time resolution for this function
unsigned long OS_MsTime(void){
return SysTickValueGet();  //returns value in SystickCounter
}

//******** OS_Launch *************** 
// start the scheduler, enable interrupts
// Inputs: number of 20ns clock cycles for each time slice
//         you may select the units of this parameter
// Outputs: none (does not return)
// In Lab 2, you can ignore the theTimeSlice field
// In Lab 3, you should implement the user-defined TimeSlice field
void OS_Launch(unsigned long theTimeSlice){
	NVIC_ST_RELOAD_R=theTimeSlice-1; //reload value
	NVIC_ST_CTRL_R = 0x00000007; //enable, core clock and interrupt arm
	StartOS();
}

// ******** OS_Thread_Init ************
// Initialize Thread Stack 
// input:  pointer to thread to initialize, value to fill in extra part of stack with
// output: none
void OS_ThreadInit(tcbType *toSet, long  filler){
	  int i;
	  //fill up extra part of the stack
	  for(i=0;i<STACKSIZE-16;i++){
	    toSet->stack[i]=filler;
	  }

	  //setup initial stack to be loaded on first run, filled with debuggin info.
	  toSet->sp = &toSet->stack[STACKSIZE-16];				// thread stack pointer
	  toSet->stack[STACKSIZE-1]= 0x01000000;				// thumb bit
	  														// R15 / PC
	  toSet->stack[STACKSIZE-3]= 0x14141414;				// R14
	  toSet->stack[STACKSIZE-4]= 0x12121212;				// R12
	  toSet->stack[STACKSIZE-5]= 0x03030303;				// R3
	  toSet->stack[STACKSIZE-6]= 0x02020202;				// R2
	  toSet->stack[STACKSIZE-7]= 0x01010101;				// R1
	  toSet->stack[STACKSIZE-8]= 0x00000000;				// R0
	  toSet->stack[STACKSIZE-9]= 0x11111111;				// R11
	  toSet->stack[STACKSIZE-10]=0x10101010;				// R10
	  toSet->stack[STACKSIZE-11]=0x09090909;				// R9
	  toSet->stack[STACKSIZE-12]=0x08080808;				// R8
	  toSet->stack[STACKSIZE-13]=0x07070707;				// R7
	  toSet->stack[STACKSIZE-14]=0x06060606;				// R6
	  toSet->stack[STACKSIZE-15]=0x05050505;				// R5
	  toSet->stack[STACKSIZE-16]=0x04040404;				// R4

	  toSet->id=0xB000B1E5;									//set initial ID to 'booobies', yes i know it's immature but i'll remember it 
	  toSet->used=0;										//mark thread as unused, ie free to be allocated
	  toSet->sleep=0;										//sleep =0
	  toSet->blockedOn=0;								//
	  toSet->realPriority=0;
	  toSet->workingPriority=0;
}

// ******** OS_SysTick_Handler ************
// Thread Switcher, uses SysTick as periodic timer, calls PendSV to actually switch threads
// Implement Thread Manager implicitly here 
// input: none, uses globals RUNPT and NEXTRUNPT 
// output: none, should switch threads when finished
void OS_SysTick_Handler(void){
	tcbType *i;
  int status;
	//Thread Scheduler
	status=StartCritical();
	for(i=RUNPT->next; i->sleep!=0 || i->blockedOn!=0; i=i->next){
		if(i->sleep>0){//decriment sleep counter
			i->sleep=i->sleep-1;
			}
	}
  
	NEXTRUNPT=i;
  
	EndCritical(status);
  
	//Switch Threads (trigger PendSV)
	NVIC_INT_CTRL_R = 0x10000000;
}

// ******** OS_SelectSwitch_Handler ************
// Check if time since last switch press >.3s, for debouncing, call buttontask appropriately
// input: none,  
// output: none, 
void OS_SelectSwitch_Handler(){
  // TODO: Button still bounces...

  GPIOPinIntDisable(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_1);
  
	//currentTime=OS_MsTime();
  while(OS_MsTime() - SDEBOUNCEPREV < 10);  // Wait for 10 ms
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) == 0){
		BUTTONTASK();	   //supposed to trigger the function that button task points to
	}	
	SDEBOUNCEPREV=OS_MsTime();
  
  GPIOPinIntEnable(GPIO_PORTF_BASE, GPIO_PIN_1);
}
