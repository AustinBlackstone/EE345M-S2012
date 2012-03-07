;Assembly code functions for operating system, contains PendSV assembly code 
;
; filename **********OS.s***********
; Real Time Operating System for Labs 2 and 3 
; Austin Blackstone, Cruz Monnreal II 2/14/2012
;***********Ready to go*************

;boiler plate, dont know why it's here, but it has to be?
        AREA    RESET, CODE, READONLY
        THUMB

;externally defined variables
   EXTERN RUNPT				 ;pointer to current thread
   EXTERN NEXTRUNPT			 ;pointer to next thread to be run


 
;******************************************************************************
;
; Useful functions.
;
;******************************************************************************

 		EXPORT  StartOS
		EXPORT  PendSV_Handler


;*********** StartOS ************************
; launces first user thread, kickstart the system
; inputs:  none
; outputs: none
StartOS
		LDR		R0, =RUNPT
		LDR		R2,	[R0]
		LDR		SP,[R2]
		POP		{R4-R11}
		POP		{R0-R3}
		POP		{R12}
		POP		{LR}
		POP		{LR}
		POP		{R1}
		CPSIE	I
		BX		LR

;*********** PendSV_handler ************************
; handles actual thread switch, pushes R4-R11 onto stack, loads thread pointed to by NEXTRUNPT  
; inputs:  none
; outputs: none
PendSV_Handler					; R0-R3, R12, LR, PC, PSR saved to stack
		CPSID	I				; interrupt disable
		PUSH	{R4-R11}   		; Save R4-R11 onto stack
		LDR		R0, =RUNPT		; R0= pointer to RUNPT, oldthread
		LDR		R1, [R0]		; R1=RUNPT
		STR		SP, [R1]		; Save SP into TCB
		LDR		R2, =NEXTRUNPT	; R2=pointer to NEXTRUNPT, newthread
		LDR		R1, [R2]		; R1=RUNPT
		STR		R1, [R0]		; RUNPT=NEXTPT
		LDR		SP, [R1]		; new thread SP; SP=RUNPT->sp
		POP		{R4-R11}		; restores R4-R11
		CPSIE	I				; interrupt enable
		BX		LR				; return, restore R0-R3, R12, LR, PC, PSR


;******************************************************************************
;
; Make sure the end of this section is aligned.
;
;******************************************************************************
        ALIGN

;******************************************************************************
;
; Tell the assembler that we're done.
;
;******************************************************************************
        END
