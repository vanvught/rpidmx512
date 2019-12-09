/**
 * @file vector.S
 *
 */
/*
 * This file contains code taken from Linux:
 *	safe_svcmode_maskall macro
 *	defined in arch/arm/include/asm/assembler.h
 *	Copyright (C) 1996-2000 Russell King
 */
 /* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

.macro FUNC name
.text
.code 32
.global \name
\name:
.endm

@ Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
.set  MODE_USR, 0x10				@ User Mode
.set  MODE_FIQ, 0x11				@ FIQ Mode
.set  MODE_IRQ, 0x12				@ IRQ Mode
.set  MODE_SVC, 0x13				@ Supervisor Mode
.set  MODE_ABT, 0x17				@ Abort Mode
.set  MODE_UND, 0x1B				@ Undefined Mode
.set  MODE_SYS, 0x1F				@ System Mode

.set  I_BIT, 0x80					@ when I bit is set, IRQ is disabled
.set  F_BIT, 0x40					@ when F bit is set, FIQ is disabled

.section .init
.code 32
.align 2
.global _start
_start:
    ldr pc, reset_handler
    ldr pc, undefined_handler
    ldr pc, swi_handler
    ldr pc, prefetch_handler
    ldr pc, data_handler
    ldr pc, unused_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler

reset_handler:		.word reset
undefined_handler:	.word und_undefined_handler
swi_handler:		.word hang
prefetch_handler:	.word abort_prefetch_handler
data_handler:		.word abort_data_handler
unused_handler:		.word hang
irq_handler:		.word irq
fiq_handler:		.word fiq

reset:
#if defined ( RPI2 ) || defined ( RPI3 )
	@ Return current CPU ID (0..3)
	mrc p15, 0, r0, c0, c0, 5 			@ r0 = Multiprocessor Affinity Register (MPIDR)
	ands r0, #3							@ r0 = CPU ID (Bits 0..1)
	bne hang 							@ If (CPU ID != 0) Branch To Infinite Loop (Core ID 1..3)
	cpsid if							@ Disable IRQ & FIQ
    @ Check for HYP mode
	mrs	r0 , cpsr
	eor	r0, r0, #0x1A
	tst	r0, #0x1F
	bic	r0 , r0 , #0x1F					@ clear mode bits
	orr	r0 , r0 , #MODE_SVC|I_BIT|F_BIT	@ mask IRQ/FIQ bits and set SVC mode
	bne	2f								@ branch if not HYP mode
	orr	r0, r0, #0x100					@ mask Abort bit
	adr	lr, 3f
	msr	spsr_cxsf, r0
	.word	0xE12EF30E					@ msr ELR_hyp, lr
	.word	0xE160006E					@ eret
2:	msr cpsr_c, r0
3:
#endif

	@ Copy vectors to 0x0000, 16 words
	mov r0, #0x0000
	ldr r1, =_start
	ldmia r1!, {r2-r9}
	stmia r0!, {r2-r9}
	ldmia r1!, {r2-r9}
	stmia r0!, {r2-r9}

    msr CPSR_c,#MODE_ABT|I_BIT|F_BIT 	@ Abort Mode
    ldr r0, =__abt_stack_top
    mov sp, r0

    msr CPSR_c,#MODE_UND|I_BIT|F_BIT 	@ Undefined Mode
    ldr r0, =__und_stack_top
    mov sp, r0

    msr CPSR_c,#MODE_IRQ|I_BIT|F_BIT 	@ IRQ Mode
    ldr r0, =__irq_stack_top
    mov sp, r0

    msr  CPSR_c,#MODE_FIQ|I_BIT|F_BIT	@ FIQ Mode
    ldr r0, =__fiq_stack_top
    mov sp, r0

    msr CPSR_c,#MODE_SVC|I_BIT|F_BIT	@ Supervisor Mode
    ldr r0, =__svc_stack_top
    mov sp, r0

    @ clear bss section
    mov   r0, #0
    ldr   r1, =__bss_start
    ldr   r2, =__bss_end
4:  cmp   r1, r2
    strlo r0, [r1], #4
    blo   4b

	bl vfp_init

#if defined ( ENABLE_MMU )
	bl mmu_enable
#else
	@ start L1 chache
	mrc p15, 0, r0, c1, c0, 0
	orr r0,r0,#0x0004			@ Data Cache (Bit 2)
	orr r0,r0,#0x0800			@ Branch Prediction (Bit 11)
	orr r0,r0,#0x1000			@ Instruction Caches (Bit 12)
	mcr p15, 0, r0, c1, c0, 0
#endif

	bl hardware_init
    bl notmain
halt:
	wfe
	b halt

und_undefined_handler:
@	msr   CPSR_c, #MODE_SVC		@ Switch to SVC mode so we can the retrieve the orignal lr
	mov   r0, #0				@ Set type parameter
	sub   r1, lr, #4;			@ Set address parameter
								@ Subtracting 4 adjusts for the instruction queue giving the address of the instruction that caused this exception
   	b    debug_exception		@ Call the debug_exception function - does not return

abort_prefetch_handler:
	msr   CPSR_c, #MODE_SVC		@ Switch to SVC mode so we can the retrieve the orignal lr
	mov   r0, #1				@ Set type parameter
	sub   r1, lr, #4;			@ Set address parameter
								@ Subtracting 4 adjusts for the instruction queue giving the address of the instruction that caused this exception
   	b    debug_exception		@ Call the debug_exception function - does not return

abort_data_handler:
	mov   r0, #2				@ Set type parameter
	sub   r1, lr, #8;			@ Set address parameter
								@ Subtracting 8 adjusts for the instruction queue giving the address of the instruction that caused this exception
	b    debug_exception		@ Call to the debug_exception function  - does not return

irq:
    b c_irq_handler				@ void __attribute__((interrupt("IRQ"))) c_irq_handler(void)

fiq:
    b c_fiq_handler				@ void __attribute__((interrupt("FIQ"))) c_fiq_handler(void)

FUNC hang
    b hang

#if defined ( RPI2 ) || defined ( RPI3 )
FUNC _init_core
#ifdef ARM_ALLOW_MULTI_CORE
    @ Check for HYP mode
	mrs	r0 , cpsr
	eor	r0, r0, #0x1A
	tst	r0, #0x1F
	bic	r0 , r0 , #0x1F					@ clear mode bits
	orr	r0 , r0 , #MODE_SVC|I_BIT|F_BIT	@ mask IRQ/FIQ bits and set SVC mode
	bne	2f								@ branch if not HYP mode
	orr	r0, r0, #0x100					@ mask Abort bit
	adr	lr, 3f
	msr	spsr_cxsf, r0
	.word	0xE12EF30E					@ msr ELR_hyp, lr
	.word	0xE160006E					@ eret
2:	msr cpsr_c, r0
3:
    msr CPSR_c,#MODE_SVC|I_BIT|F_BIT	@ Supervisor Mode

	@ Return current CPU ID (0..3)
	mrc p15, 0, r0, c0, c0, 5 			@ r0 = Multiprocessor Affinity Register (MPIDR)
	ands r0, #3							@ r0 = CPU ID (Bits 0..1)

	cmp r0, #1							@ CPU ID == 1
    ldreq r0, =__svc_stack_top_core1
    beq 4f
    cmp r0, #2							@ CPU ID == 2
    ldreq r0, =__svc_stack_top_core2
    beq 4f
    ldr r0, =__svc_stack_top_core3		@ CPU ID == 3
4:	mov sp, r0

	bl vfp_init
	bl mmu_enable

	ldr r3, =smp_core_main
    blx r3
#else
	dsb
1:	wfi
	b	1b
#endif

#endif
