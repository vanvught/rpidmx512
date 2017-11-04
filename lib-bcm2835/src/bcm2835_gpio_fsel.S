/**
 * @file bcm2835_gpio_fsel.S
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "bcm2835.h"

.macro FUNC name
.text
.code 32
.global \name
\name:
.endm

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
mov r2, #7
lsl r2, r3
lsl r1, r3
ldr	r3, [r0]				
and	r1, r2, r1
bic	r3, r3, r2
orr	r3, r3, r1
str	r3, [r0]
pop {pc}
