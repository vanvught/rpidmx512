/**
 * @file arm.h
 *
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

#ifndef ARM_H_
#define ARM_H_

#include <stdbool.h>
#include <stddef.h>

typedef enum arm_vector {
	ARM_VECTOR_RESET = 0x00,			///< Reset
	ARM_VECTOR_UNDEF = 0x04,			///< Undefined Instruction
	ARM_VECTOR_SWI = 0x08,				///< Software Interrupt (SWI)
	ARM_VECTOR_PREFETCH_ABORT = 0x0C,	///< Prefetch Abort
	ARM_VECTOR_DATA_ABORT = 0x10,		///< Data Abort
	ARM_VECTOR_RESERVED = 0x14,			///< Reserved
	ARM_VECTOR_IRQ = 0x18,				///< Interrupt (IRQ)
	ARM_VECTOR_FIQ = 0x1C				///< Fast Interrupt (FIQ)
} _arm_vector;

#define ARM_VECTOR(x)	(unsigned *)(x)

#define	__enable_irq()	asm volatile ("cpsie i")
#define	__disable_irq()	asm volatile ("cpsid i")
#define	__enable_fiq()	asm volatile ("cpsie f")
#define	__disable_fiq()	asm volatile ("cpsid f")

#ifdef __cplusplus
extern "C" {
#endif

extern bool arm_install_handler(unsigned routine, unsigned *vector);

extern void arm_dump_vector_table(void);
extern void arm_dump_page_table(void);

/**
 * Copy 8 words = 32 bytes
 */
extern void *memcpy_blk(void *, const void *, size_t);

#ifdef __cplusplus
}
#endif

#endif /* ARM_H_ */
