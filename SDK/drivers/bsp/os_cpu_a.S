!********************************************************************************************************
!                                               FreeRTOS
!
!                               (c) Copyright 2016, Andes Techonology
!                                          All Rights Reserved
!
!                                           NDS32 Generic Port
!                                            GNU C Compiler
!
!********************************************************************************************************
!********************************************************************************************************
! INCLUDE ASSEMBLY CONSTANTS
!********************************************************************************************************

#include "nds32.h"
#include "FreeRTOSConfig.h"
#include "os_cpu_common.h"
#include "irq.h"

!********************************************************************************************************
! IMPORT and EXPORT
!********************************************************************************************************

	.global CtxRestore
	.global __OS_Int_Stack

	.macro TCBSaveMacro
		! Store the new top of stack for the task.
		la	$r3, pxCurrentTCB	! pxCurrentTCB->pxTopOfStack = $SP
		lwi	$r4, [$r3]
		swi	$sp, [$r4]
	.endm

	! Restore to new task's context
	.macro TCBRestoreMacro
		! use new pxCurrentTCB to get new stack pointer
		la	$r0, pxCurrentTCB
		lwi	$r1, [$r0]
		lwi	$sp, [$r1]		! $SP = pxCurrentTCB->pxTopOfStack
	.endm

!*************************************************************************
!
!	PortMacro Function, Call by C function
!	Used by First start Scheduler
!	Warm up a Initalized Task
!
!*************************************************************************
	! Restore to new task's context
CtxRestore:
	TCBRestoreMacro
	RESTORE_ALL
	iret

!********************************************************************************************************
!                   PERFORM A CONTEXT SWITCH - OS_Trap_Interrupt_SWI()
!
! Note(s) : 1) Software interrupt handler performs the actual context switch
!
!           2) The pseudo-code for OSCtxSw() is:
!              a) $IPSW <- $PSW, $IPC <- $lp
!              b) SAVE_ALL              : Save current task's context
!              c) TCBSaveMacro          : Save current task's sp into TCB
!              d) vTaskSwitchContext    : get the highest priority of Task
!              e) TCBRestoreMacro       : Restore to the highest priority task's sp from TCB
!              f) RESTORE_ALL           : Restore to the highest priority task's context
!
!********************************************************************************************************

OS_Trap_Interrupt_SWI:
	SAVE_ALL

	IntlDescend                         ! Descend interrupt level

	movi	$r1, 0x0
	mtsr	$r1, $INT_PEND              ! clean SWI pending

	TCBSaveMacro                        ! Save current task's sp into TCB

	la	$sp, __OS_Int_Stack             ! switch to interrupt stack

	/*
	 * TODO :
	 * We can set the interrupt priority to allow those interrupts running at higher
	 * priorities to come in and will never be delayed. In other word, interrupts that
	 * do NOT call any FreeRTOS kernel API functions is allowed and the interrupts that
	 * call API functions ending in "FromISR" must be disabled.
	 * It can be implemented by "configKERNEL_INTERRUPT_PRIORITY" and
	 * "configMAX_SYSCALL_INTERRUPT_PRIORITY"
	 */

	bal	vTaskSwitchContext              ! get the highest priority of Task

	TCBRestoreMacro                     ! Restore to original task's TCB

	RESTORE_ALL

	iret


!********************************************************************************************************
! IRQ Interrupt Service Routine
!********************************************************************************************************

	.global OS_Trap_Int_Comm

	! Set up Interrupt vector ISR
	! HW#IRQ_SWI_VECTOR : OS_Trap_Interrupt_SWI (SWI)
	! HW#n : OS_Trap_Int_Comm

	.macro	SET_HWISR num
	.global OS_Trap_Interrupt_HW\num
	.if	\num == IRQ_VECTOR_SWI
	.set	OS_Trap_Interrupt_HW\num, OS_Trap_Interrupt_SWI
	.else
	.set	OS_Trap_Interrupt_HW\num, OS_Trap_Int_Comm
	.endif
	.endm

	.altmacro
	.set	irqno, 0
	.rept	VECTOR_NUMINTRS
	SET_HWISR %irqno
	.set	irqno, irqno+1
	.endr
	.noaltmacro


OS_Trap_Int_Comm:
	! $r0 : HW Interrupt vector number
	SAVE_CALLER

	IntlDescend                             ! Descend interrupt level

	mfsr	$r1, $IPSW                      ! Use IPSW.CPL to check come from thread or ISR
	srli45	$r1, #PSW_offCPL
	fexti33	$r1, #0x2                       ! IPSW.CPL
	bnec	$r1, #0x7, 2f                   ! IPSW.CPL != 7, come form ISR, reentrant

	move	$fp, $sp                        ! save old stack pointer
	la	$sp, __OS_Int_Stack             ! switch to interrupt stack
2:
#if 0
	setgie.e                                ! allow nested now
#endif
	! The entire CPU state is now stashed on the stack,
	! and the stack is also 8-byte alignment.
	! We can call C program based interrupt handler now.
	la	$r1, OS_CPU_Vector_Table
	lw	$r1, [$r1+($r0<<2)]             ! ISR function pointer
	jral	$r1                             ! Call ISR

	la	$r1, __OS_Int_Stack             ! Check for nested interruption return
	bne	$r1, $sp, 3f                    ! $sp != __OS_Int_Stack?

	move	$sp, $fp                        ! Move back to the thread stack
3:
	RESTORE_CALLER

	iret


!********************************************************************************************************
! POINTERS TO VARIABLES
!********************************************************************************************************
	.section .bss
	.skip IRQ_STACK_SIZE
	.align 3
__OS_Int_Stack:

	.end
