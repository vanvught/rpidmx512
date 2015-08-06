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
undefined_handler:	.word hang
swi_handler:		.word hang
prefetch_handler:	.word hang
data_handler:		.word hang
unused_handler:		.word hang
irq_handler:		.word irq
fiq_handler:		.word fiq

reset:
    @ Copy vectors (including fiq handler) to 0x0000
    ldr   r1, =_start
    mov   r2, #0x0000
    ldr   r3, =reset_handler
1:  cmp   r2, r3
    ldrlo r0, [r1], #4
    strlo r0, [r2], #4
    blo   1b

    msr CPSR_c,#MODE_IRQ|I_BIT|F_BIT 	@ IRQ Mode
    ldr r0, =__irq_stack_top
    mov sp, r0

    msr  CPSR_c,#MODE_FIQ|I_BIT|F_BIT	@ FIQ Mode
    ldr r0, =__fiq_stack_top
    mov sp, r0

    msr CPSR_c,#MODE_SVC|I_BIT|F_BIT	@ Supervisor Mode
    ldr r0, =__svc_stack_top
    mov sp, r0

	@ start L1 chache
	mrc p15, 0, r0, c1, c0, 0
	orr r0,r0,#0x0004					@ Data Cache (Bit 2)
	orr r0,r0,#0x0800					@ Branch Prediction (Bit 11)
	orr r0,r0,#0x1000					@ Instruction Caches (Bit 12)
	mcr p15, 0, r0, c1, c0, 0

    @ Enable fpu
    mrc p15, 0, r0, c1, c0, 2
    orr r0,r0,#0x300000 				@ single precision
    orr r0,r0,#0xC00000 				@ double precision
    mcr p15, 0, r0, c1, c0, 2
    mov r0,#0x40000000
    fmxr fpexc,r0

    @ clear bss section
    mov   r0, #0
    ldr   r1, =__bss_start
    ldr   r2, =__bss_end
2:  cmp   r1, r2
    strlo r0, [r1], #4
    blo   2b

    bl notmain
halt:
	wfe
	b halt

irq:
    b c_irq_handler			@ void __attribute__((interrupt("IRQ"))) c_irq_handler(void)

fiq:
    b c_fiq_handler			@ void __attribute__((interrupt("FIQ"))) c_fiq_handler(void)

FUNC hang
    b hang

FUNC __enable_irq
    mrs r0, cpsr
    bic r0, r0, #I_BIT
    msr cpsr_c, r0
    bx lr

FUNC __disable_irq
    mrs r1, CPSR
    orr r1, r1, #I_BIT
    msr cpsr_c, r1
    bx lr

FUNC __enable_fiq
    mrs r0, cpsr
    bic r0, r0, #F_BIT
    msr cpsr_c, r0
    bx lr

FUNC __disable_fiq
    mrs r1, cpsr
    orr r1, r1, #F_BIT
    msr cpsr_c, r1
    bx lr

FUNC memory_barrier
#if defined ( RPI2 )
	dmb
#else
    mov r0, #0
    mcr p15, #0, r0, c7, c10, #5
#endif
    bx lr
