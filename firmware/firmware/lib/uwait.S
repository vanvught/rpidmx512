#include <bcm2835.h>

.macro FUNC name
.text
.code 32
.global \name
\name:
.endm

.cpu arm1176jzf-s
.fpu vfp

FUNC uwait
	push {r4, r5, lr}
	mov r5,r0
	ldr r4,=BCM2835_ST_BASE
    ldrd r2,r3,[r4,#4]
4:  ldrd r0,r1,[r4,#4]
    sub r1,r0,r2
    cmp r1,r5
    bls 4b
    pop {r4, r5, pc}
