//*****************************************************************************
//
// Lab2.c - user programs
// Jonathan Valvano, Feb 4, 2011, EE345M
//
//*****************************************************************************
// feel free to adjust these includes as needed
#include <stdio.h>
#include <string.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
//#include "serial.h"
//#include "uart_echo_mod.h"
#include "rit128x96x4.h"
#include "adc.h"
#include "os.h"
#include "gpio.h"
#include "uart.h"
#include "inc/lm3s8962.h"
#include "driverlib/sysctl.h"

unsigned long NumCreated;   // number of foreground threads created
unsigned long PIDWork;      // current number of PID calculations finished
unsigned long FilterWork;   // number of digital filter calculations finished
unsigned long NumSamples;   // incremented every sample
unsigned long DataLost;     // data sent by Producer, but not received by Consumer
long MaxJitter;             // largest time jitter between interrupts in usec
long MinJitter;             // smallest time jitter between interrupts in usec
#define JITTERSIZE 64
unsigned long const JitterSize=JITTERSIZE;
unsigned long JitterHistogram[JITTERSIZE]={0,};

#define TIMESLICE TIME_1MS*2  // thread switch time in system time units
#define PERIOD    TIME_1MS/2  // 2kHz sampling period in system time units
// 10-sec finite time experiment duration 
#define RUNLENGTH 10000   // display results and quit when NumSamples==RUNLENGTH
long x[64],y[64];         // input and output arrays for FFT
void cr4_fft_64_stm32(void *pssOUT, void *pssIN, unsigned short Nbin);

extern int OSMAILBOX;

//------------------Task 1--------------------------------
// 2 kHz sampling ADC channel 1, using software start trigger
// background thread executed at 2 kHz
// 60-Hz notch IIR filter, assuming fs=2000 Hz
// y(n) = (256x(n) -503x(n-1) + 256x(n-2) + 498y(n-1)-251y(n-2))/256
short Filter(short data){
static short x[6]; // this MACQ needs twice
static short y[6];
static unsigned int n=3;   // 3, 4, or 5
  n++;
  if(n==6) n=3;     
  x[n] = x[n-3] = data;  // two copies of new data
  y[n] = (256*(x[n]+x[n-2])-503*x[1]+498*y[1]-251*y[n-2]+128)/256;
  y[n-3] = y[n];         // two copies of filter outputs too
  return y[n];
} 
//******** DAS *************** 
// background thread, calculates 60Hz notch filter
// runs 2000 times/sec
// inputs:  none
// outputs: none
unsigned short DASoutput;

void DAS(void){ 
int index;
unsigned short input;  
unsigned static long LastTime;  // time at previous ADC sample
unsigned long thisTime;         // time at current ADC sample
long jitter;                    // time between measured and expected
  if(NumSamples < RUNLENGTH){   // finite time run
    input = ADC_Read(1);
    thisTime = OS_Time();       // current time, 20 ns
    DASoutput = Filter(input);
    FilterWork++;        // calculation finished
    if(FilterWork>1){    // ignore timing of first interrupt
      jitter = OS_TimeDifference(thisTime,LastTime)/50-PERIOD/50;  // in usec
      if(jitter > MaxJitter){
        MaxJitter = jitter;
      }
      if(jitter < MinJitter){
        MinJitter = jitter;
      }        // jitter should be 0
      index = jitter+JITTERSIZE/2;   // us units
      if(index<0)index = 0;
      if(index>=JitterSize) index = JITTERSIZE-1;
      JitterHistogram[index]++; 
    }
    LastTime = thisTime;
  }
}
//--------------end of Task 1-----------------------------

//------------------Task 2--------------------------------
// background thread executes with select button
// one foreground task created with button push
// foreground treads run for 2 sec and die
// ***********ButtonWork*************
void ButtonWork(void){
unsigned long i;
unsigned long myId = OS_Id(); 
  oLED_Message(1,0,"NumCreated = ",NumCreated); 
  if(NumSamples < RUNLENGTH){   // finite time run
    ;//for(i=0;i<10;i++){  // runs for 2 seconds
    //  OS_Sleep(1);     // set this to sleep for 0.1 sec
    //}
  }
  oLED_Message(1,1,"PIDWork    =", PIDWork);
  oLED_Message(1,2,"DataLost   =", DataLost);
  oLED_Message(1,3,"Jitter(us) =", MaxJitter-MinJitter);
  OS_Kill();  // done
} 

//************ButtonPush*************
// Called when Select Button pushed
// Adds another foreground task
// background threads execute once and return
void ButtonPush(void){
  if(OS_AddThread(&ButtonWork,100,4)){
    NumCreated++; 
  }
}
//--------------end of Task 2-----------------------------

//------------------Task 3--------------------------------
// hardware timer-triggered ADC sampling at 1 kHz
// Producer runs as part of ADC ISR
// Producer uses fifo to transmit 1000 samples/sec to Consumer
// every 64 samples, Consumer calculates FFT
// every 64 ms, consumer sends data to Display via mailbox
// Display thread updates oLED with measurement

//******** Producer *************** 
// The Producer in this lab will be called from your ADC ISR
// A timer runs at 1 kHz, started by your ADC_Collect
// The timer triggers the ADC, creating the 1 kHz sampling
// Your ADC ISR runs when ADC data is ready
// Your ADC ISR calls this function with a 10-bit sample 
// sends data to the consumer, runs periodically at 1 kHz
// inputs:  none
// outputs: none
void Producer(unsigned short data){  
  if(NumSamples < RUNLENGTH){   // finite time run
    NumSamples++;               // number of samples
    if(OS_Fifo_Put(data) == 0){ // send to consumer
      DataLost++;
    } 
  } 
}

//******** Display *************** 
// foreground thread, accepts data from consumer
// displays calculated results on the LCD
// inputs:  none                            
// outputs: none
void Display(void){ 
  unsigned long data, voltage;
  
  oLED_Message(0,0,"Run length is",(RUNLENGTH)/1000);   // top half used for Display
  while(NumSamples < RUNLENGTH) { 
    oLED_Message(0,1,"Time left is",(RUNLENGTH-NumSamples)/1000);   // top half used for Display
    data = OS_MailBox_Recv();
    voltage = 3000*data/1024;               // calibrate your device so voltage is in mV
    oLED_Message(0,2,"v(mV) =",voltage);
  } 
  OS_Kill();  // done
} 
//******** Consumer *************** 
// foreground thread, accepts data from producer
// calculates FFT, sends DC component to Display
// inputs:  none
// outputs: none
void Consumer(void){ 
  unsigned long data, DCcomponent; // 10-bit raw ADC sample, 0 to 1023
  unsigned long t;  // time in ms
  unsigned long myId = OS_Id();
  
  NumCreated += OS_AddThread(&Display,128,0); 
  while(NumSamples < RUNLENGTH) { 
    ADC_Collect(0, 1000, &Producer, 64); // start ADC sampling, channel 0, 1 kHz, 64 samples
    for(t = 0; t < 64; t++){   // collect 64 ADC samples
      data = OS_Fifo_Get();    // get from producer
      x[t] = data;             // real part is 0 to 1023, imaginary part is 0
    }
    cr4_fft_64_stm32(y,x,64);  // complex FFT of last 64 ADC values
    DCcomponent = y[0]&0xFFFF; // Real part at frequency 0, imaginary part should be zero
    OS_MailBox_Send(DCcomponent);
  }
  OS_Kill();  // done
}

//--------------end of Task 3-----------------------------

//------------------Task 4--------------------------------
// foreground thread that runs without waiting or sleeping
// it executes a digital controller 
//******** PID *************** 
// foreground thread, runs a PID controller
// never blocks, never sleeps, never dies
// inputs:  none
// outputs: none
short IntTerm;     // accumulated error, RPM-sec
short PrevError;   // previous error, RPM
short Coeff[3];    // PID coefficients
short PID_stm32(short Error, short *Coeff);
short Actuator;
void PID(void){ 
short err;  // speed error, range -100 to 100 RPM
unsigned long myId = OS_Id(); 
  PIDWork = 0;
  IntTerm = 0;
  PrevError = 0;
  Coeff[0] = 384;   // 1.5 = 384/256 proportional coefficient
  Coeff[1] = 128;   // 0.5 = 128/256 integral coefficient
  Coeff[2] = 64;    // 0.25 = 64/256 derivative coefficient*
  while(NumSamples < RUNLENGTH) { 
    for(err = -1000; err <= 1000; err++){    // made-up data
      Actuator = PID_stm32(err,Coeff)/256;
    }
    PIDWork++;        // calculation finished
  }
  for(;;){ }          // done
}
//--------------end of Task 4-----------------------------

//------------------Task 5--------------------------------
// UART background ISR performs serial input/output
// two fifos are used to pass I/O data to foreground
// Lab 1 interpreter runs as a foreground thread
// the UART driver should call OS_Wait(&RxDataAvailable) when foreground tries to receive
// the UART ISR should call OS_Signal(&RxDataAvailable) when it receives data from Rx
// similarly, the transmit channel waits on a semaphore in the foreground
// and the UART ISR signals this semaphore (TxRoomLeft) when getting data from fifo
// it executes a digital controller 
// your intepreter from Lab 1, with additional commands to help debug 
// foreground thread, accepts input from serial port, outputs to serial port
// inputs:  none
// outputs: none
void Interpreter(void){ // Pipe to UART Parser (localvars in uart.c, etc...)
  while(1){
    UARTParse();
  }
}

// add the following commands, leave other commands, if they make sense
// 1) print performance measures 
//    time-jitter, number of data points lost, number of calculations performed
//    i.e., NumSamples, NumCreated, MaxJitter-MinJitter, DataLost, FilterWork, PIDwork
      
// 2) print debugging parameters 
//    i.e., x[], y[] 
//--------------end of Task 5-----------------------------

//*******************final user main DEMONTRATE THIS TO TA**********
int main(void){ 
  OS_Init();           // initialize, disable interrupts

  DataLost = 0;        // lost data between producer and consumer
  NumSamples = 0;
  MaxJitter = 0;       // OS_Time in 20ns units
  MinJitter = 10000000;

//********initialize communication channels
  OS_MailBox_Init();
  OS_Fifo_Init(8);    // ***note*** 4 is not big enough*****

//*******attach background tasks***********
  OS_AddButtonTask(&ButtonPush,2);
  OS_AddPeriodicThread(&DAS,PERIOD,1); // 2 kHz real time sampling

  NumCreated = 0 ;
// create initial foreground threads
  NumCreated += OS_AddThread(&Interpreter,128,2); 
  NumCreated += OS_AddThread(&Consumer,128,1);
  NumCreated += OS_AddThread(&PID,128,3); 
 
  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;             // this never executes
}

//+++++++++++++++++++++++++DEBUGGING CODE++++++++++++++++++++++++
// ONCE YOUR RTOS WORKS YOU CAN COMMENT OUT THE REMAINING CODE
// 
//*******************Initial TEST**********
// This is the simplest configuration, test this first
// run this with 
// no UART interrupts
// no SYSTICK interrupts
// no timer interrupts
// no select interrupts
// no ADC serial port or oLED output
// no calls to semaphores
unsigned long Count1;   // number of times thread1 loops
unsigned long Count2;   // number of times thread2 loops
unsigned long Count3;   // number of times thread3 loops
unsigned long Count4;   // number of times thread4 loops
unsigned long Count5;   // number of times thread5 loops
/*void Thread1(void){
  Count1 = 0;          
  for(;;){
    Count1++;
    OS_Suspend();      // cooperative multitasking
  }
}
void Thread2(void){
  Count2 = 0;          
  for(;;){
    Count2++;
    OS_Suspend();      // cooperative multitasking
  }
}
void Thread3(void){
  Count3 = 0;          
  for(;;){
    Count3++;
    OS_Suspend();      // cooperative multitasking
  }
}
Sema4Type Free;       // used for mutual exclusion

int main1(void){ 
  OS_Init();           // initialize, disable interrupts
  NumCreated = 0 ;
  NumCreated += OS_AddThread(&Thread1,128,1); 
  NumCreated += OS_AddThread(&Thread2,128,2); 
  NumCreated += OS_AddThread(&Thread3,128,3); 
 
  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;             // this never executes
}

// *******************Second TEST**********
// Once the initalize test runs, test this 
// no UART interrupts
// SYSTICK interrupts, with or without period established by OS_Launch
// no timer interrupts
// no select switch interrupts
// no ADC serial port or oLED output
// no calls to semaphores
void Thread1b(void){
  Count1 = 0;          
  for(;;){
    Count1++;
  }
}
void Thread2b(void){
  Count2 = 0;          
  for(;;){
    Count2++;
  }
}
void Thread3b(void){
  Count3 = 0;          
  for(;;){
    Count3++;
  }
}
int main2(void){  // testmain2
  OS_Init();           // initialize, disable interrupts
  NumCreated = 0 ;
  NumCreated += OS_AddThread(&Thread1b,128,1); 
  NumCreated += OS_AddThread(&Thread2b,128,2); 
  NumCreated += OS_AddThread(&Thread3b,128,3); 
 
  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;             // this never executes
}

//*******************Third TEST**********
// Once the second test runs, test this 
// no UART1 interrupts
// SYSTICK interrupts, with or without period established by OS_Launch
// Timer2 interrupts, with or without period established by OS_AddPeriodicThread
// PortF GPIO interrupts, active low
// no ADC serial port or oLED output
// tests the spinlock semaphores, tests Sleep and Kill
Sema4Type Readyc;        // set in background
int Lost;
void BackgroundThread1c(void){   // called at 1000 Hz
  Count1++;
  OS_Signal(&Readyc);
}
void Thread5c(void){
  for(;;){
    OS_Wait(&Readyc);
    Count5++;   // Count2 + Count5 should equal Count1 
    Lost = Count1-Count5-Count2;
  }
}
void Thread2c(void){
  OS_InitSemaphore(&Readyc,0);
  Count1 = 0;    // number of times signal is called      
  Count2 = 0;    
  Count5 = 0;    // Count2 + Count5 should equal Count1  
  NumCreated += OS_AddThread(&Thread5c,128,3); 
  OS_AddPeriodicThread(&BackgroundThread1c, SysCtlClockGet()/1000, 0); 
  for(;;){
    OS_Wait(&Readyc);
    Count2++;   // Count2 + Count5 should equal Count1
  }
}

void Thread3c(void){
  Count3 = 0;          
  for(;;){
    Count3++;
  }
}
void Thread4c(void){ int i;
  //for(i=0;i<64;i++){
    Count4++;
    
    //OS_Sleep(10);
  //}
  OS_Kill();
  //Count4 = 0;
}
void BackgroundThread5c(void){   // called when Select button pushed
  NumCreated += OS_AddThread(&Thread4c,128,3); 
}
      
int main3(void){   // Testmain3
  Count4 = 0;          
  OS_Init();           // initialize, disable interrupts

  NumCreated = 0 ;
  OS_AddButtonTask(&BackgroundThread5c,2);
  NumCreated += OS_AddThread(&Thread2c,128,2); 
  NumCreated += OS_AddThread(&Thread3c,128,3); 
  //NumCreated += OS_AddThread(&Thread4c,128,3); 
  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;  // this never executes
}


//*******************Fourth TEST**********
// Once the third test runs, run this example
// Count1 should exactly equal Count2
// Count3 should be very large
// Count4 increases by 640 every time select is pressed
// NumCreated increase by 1 every time select is pressed

// no UART interrupts
// SYSTICK interrupts, with or without period established by OS_Launch
// Timer interrupts, with or without period established by OS_AddPeriodicThread
// Select switch interrupts, active low
// no ADC serial port or oLED output
// tests the spinlock semaphores, tests Sleep and Kill
Sema4Type Readyd;        // set in background
void BackgroundThread1d(void){   // called at 1000 Hz
static int i=0;
  i++;
  if(i==50){
    i = 0;         //every 50 ms
    Count1++;
    OS_bSignal(&Readyd);
  }
}
void Thread2d(void){
  OS_InitSemaphore(&Readyd,0);
  Count1 = 0;          
  Count2 = 0;          
  for(;;){
    OS_bWait(&Readyd);
    Count2++;     
  }
}
void Thread3d(void){
  Count3 = 0;          
  for(;;){
    Count3++;
  }
}
void Thread4d(void){
  int i;
    
  for(i=0;i<640;i++){
    Count4++;
    OS_Sleep(1);
  }
  OS_Kill();
}
void BackgroundThread5d(void){   // called when Select button pushed
  NumCreated += OS_AddThread(&Thread4d,128,3); 
}
int main4(void){   // Testmain4
  Count4 = 0;          
  OS_Init();           // initialize, disable interrupts
  NumCreated = 0 ;
  OS_AddPeriodicThread(&BackgroundThread1d,PERIOD,0); 
  OS_AddButtonTask(&BackgroundThread5d,2);
  NumCreated += OS_AddThread(&Thread2d,128,2); 
  NumCreated += OS_AddThread(&Thread3d,128,3); 
  NumCreated += OS_AddThread(&Thread4d,128,3); 
  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;  // this never executes
}
*/

void main5(void){
OS_Init();
OS_AddThread(&Interpreter,128,2);
OS_Launch(TIMESLICE);
return;
}
