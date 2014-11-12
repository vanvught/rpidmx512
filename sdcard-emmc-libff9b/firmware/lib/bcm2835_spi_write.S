#include "bcm2835.h"
#include "bcm2835_spi.h"

.macro FUNC name
.text
.code 32
.global \name
\name:
.endm

.cpu arm1176jzf-s
.fpu vfp

FUNC bcm2835_spi_write
@ void bcm2835_spi_write(uint16_t data)
ldr	r3, =BCM2835_SPI0_BASE
ldr	r1, [r3]
orr	r1, r1, #BCM2835_SPI0_CS_CLEAR
str	r1, [r3]
ldr	r1, [r3]
orr	r1, r1, #BCM2835_SPI0_CS_TA
str	r1, [r3]
cs_txd_loop:
    ldr	r1, [r3]
    tst	r1, #BCM2835_SPI0_CS_TXD
    beq	cs_txd_loop
lsr	r1, r0, #8
uxtb	r0, r0
str	r1, [r3, #BCM2835_SPI0_FIFO]
str	r0, [r3, #BCM2835_SPI0_FIFO]
cs_done:
 	ldr	r1, [r3]
 	tst	r1, #BCM2835_SPI0_CS_DONE
 	beq	cs_done
ldr	r1, [r3]
bic	r1, r1, #BCM2835_SPI0_CS_TA
str	r1, [r3]
bx lr
