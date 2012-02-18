#ifndef __Startup_H
#define __Startup_H 1

void DisableInterrupts(void);  
void EnableInterrupts(void);
long StartCritical(void);
void EndCritical(long);
void StartOS(void);

#endif