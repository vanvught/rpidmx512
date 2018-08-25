/**
 * @file vector.S
 *
 */
 /* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

.set  I_BIT, 0x80				@ when I bit is set, IRQ is disabled
.set  F_BIT, 0x40				@ when F bit is set, FIQ is disabled

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
    @set VBAR
    ldr   r0, =_start
    mcr   p15, 0, r0, c12, c0, 0

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

    cpsid i
    cpsid f

    @ clear bss section
    mov   r0, #0
    ldr   r1, =__bss_start
    ldr   r2, =__bss_end
4:  cmp   r1, r2
    strlo r0, [r1], #4
    blo   4b

    bl vfp_init

	@ Bit 2 is set by U-Boot
    mrc p15, 0, r0, c1, c0, 0
	bic r0,r0, #0x0002 			@ Allow misalignment (Bit 2)
    mcr p15, 0, r0, c1, c0, 0

#if defined ( ENABLE_MMU )
	bl mmu_enable
#else
	@ start L1 chache
	mrc p15, 0, r0, c1, c0, 0
	orr r0,r0, #0x0004			@ Data Cache (Bit 3)
	orr r0,r0, #0x0800			@ Branch Prediction (Bit 11)
	orr r0,r0, #0x1000			@ Instruction Caches (Bit 12)
	mcr p15, 0, r0, c1, c0, 0
#endif

    bl hardware_init

#if defined ( NO_EMAC )
	bl emac_shutdown
#endif

    bl notmain
halt:
    wfe
    b halt

und_undefined_handler:
    mov   r0, #0			@ Set type parameter
    sub   r1, lr, #4;		@ Set address parameter
							@ Subtracting 4 adjusts for the instruction queue giving the address of the instruction that caused this exception
    b    debug_exception	@ Call the debug_exception function - does not return

abort_prefetch_handler:
    msr   CPSR_c, #MODE_SVC	@ Switch to SVC mode so we can the retrieve the orignal lr
    mov   r0, #1			@ Set type parameter
    sub   r1, lr, #4;		@ Set address parameter
							@ Subtracting 4 adjusts for the instruction queue giving the address of the instruction that caused this exception
    b    debug_exception	@ Call the debug_exception function - does not return

abort_data_handler:
    mov   r0, #2			@ Set type parameter
    sub   r1, lr, #8;		@ Set address parameter
							@ Subtracting 8 adjusts for the instruction queue giving the address of the instruction that caused this exception
    b    debug_exception	@ Call to the debug_exception function  - does not return

irq:
    b c_irq_handler			@ void __attribute__((weak)) __attribute__((interrupt("IRQ"))) c_irq_handler(void)

fiq:
    b c_fiq_handler			@ void __attribute__((weak)) __attribute__((interrupt("FIQ"))) c_fiq_handler(void)


FUNC hang
    b hang
