#include "bcm2835.h"

.macro FUNC name
.text
.code 32
.global \name
\name:
.endm

.cpu arm1176jzf-s
.fpu vfp

FUNC bcm2835_gpio_fsel
@ void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode)
push {lr}
mov r3, r0
ldr r0, =BCM2835_GPIO_BASE
1:							@ Baking Pi Lesson 3 OK03
	cmp r3,#9				@ http://www.cl.cam.ac.uk/freshers/raspberrypi/tutorials/os/ok03.html
	subhi r3,#10
	addhi r0,#4
	bhi 1b					
add r3, r3, lsl #1			
mov r2, #7					@ r2 = BCM2835_GPIO_FSEL_MASK
lsl r2, r3					@ r2 = BCM2835_GPIO_FSEL_MASK << r3
lsl r1, r3					@ r1 = mode << r3
ldr	r3, [r0]				
and	r1, r2, r1
bic	r3, r3, r2
orr	r3, r3, r1
str	r3, [r0]
pop {pc}

FUNC bcm2835_st_delay
@ void bcm2835_st_delay(uint64_t offset_micros, uint64_t micros)
push {r3, r4, r5, lr}
adds r4, r2, r0
adc	r5, r3, r1
ldr r2, =BCM2835_ST_BASE
2:
	ldrd r0,r1,[r2,#4]
	cmp	r1, r5
	cmpeq r0, r4
	bcc	2b
pop	{r3, r4, r5, pc}

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
