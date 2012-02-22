// filename **********uart_echo_mod.h***********
// Real Time Operating System for Labs 2+
// contains header for modified uart echo file that is used for the UART functions
// Created 2/19/2012
// last modified: 2/19/2012
// Austin Blackstone, Cruz Monnreal II

#ifndef __UART_ECHO_MOD_H
#define __UART_ECHO_MOD_H 1

// ******** UARTInit ************
// Initializes UART0 (pin A) to be used as the UART for taking commands
// Initializes UART FIFO's (in, out, command)
// input:  none
// output: none
void UARTInit(void);

// ******** UARTIntHandler ************
// puts all characters from hardware to software UART FIFO
// gets all characters from software to hardware UART FIFO
// can be called externally whenever you need to trigger output of text
// input:  none
// output: none
void UARTIntHandler(void);





#endif
