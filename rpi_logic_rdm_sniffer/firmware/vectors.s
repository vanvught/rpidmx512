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
#if defined ( RPI2 )
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

#if defined ( RPI2 )
	bl mmu_enable
#else
    @ start L1 chache
    mrc p15, 0, r0, c1, c0, 0
    orr r0,r0,#0x0004					@ Data Cache (Bit 2)
    orr r0,r0,#0x0800					@ Branch Prediction (Bit 11)
    orr r0,r0,#0x1000					@ Instruction Caches (Bit 12)
    mcr p15, 0, r0, c1, c0, 0
#endif

    @ Enable fpu
    mrc p15, 0, r0, c1, c0, 2			@ Read Coprocessor Access Control Register
    orr r0,r0,#0x300000 				@ bit 20/21, Full Access, CP10
    orr r0,r0,#0xC00000 				@ bit 22/23, Full Access, CP11
    mcr p15, 0, r0, c1, c0, 2			@ Write Coprocessor Access Control Register
#if defined ( RPI2 )
    isb
#else
    mov r0,#0
    mcr p15, #0, r0, c7, c5,  #4
#endif
    mov r0,#0x40000000
#if defined ( RPI2 )
    vmsr fpexc, r0
#else
    fmxr fpexc, r0
#endif

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

#if defined ( RPI2 )
FUNC _init_core
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

	bl mmu_enable

    @ Enable fpu
    mrc p15, 0, r0, c1, c0, 2			@ Read Coprocessor Access Control Register
    orr r0,r0,#0x300000 				@ bit 20/21, Full Access, CP10
    orr r0,r0,#0xC00000 				@ bit 22/23, Full Access, CP11
    mcr p15, 0, r0, c1, c0, 2			@ Write Coprocessor Access Control Register
    isb
    mov r0,#0x40000000
    vmsr fpexc, r0

	ldr r3, =smp_core_main
    blx r3
halt_core:
	wfe
	b halt_core
#endif
