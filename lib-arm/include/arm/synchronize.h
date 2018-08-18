/**
 * @file synchronize.h
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

#ifndef SYNCHRONIZE_H_
#define SYNCHRONIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined( RPI1 )
	#define isb() asm volatile ("mcr p15, #0, %[zero], c7, c5,  #4" : : [zero] "r" (0) )
	#define dsb() asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) )
	#define dmb() asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0) )

	#define invalidate_instruction_cache()	asm volatile ("mcr p15, #0, %[zero], c7, c5,  #0" : : [zero] "r" (0) )
	#define flush_prefetch_buffer()			asm volatile ("mcr p15, #0, %[zero], c7, c5,  #4" : : [zero] "r" (0) )
	#define flush_branch_target_cache()		asm volatile ("mcr p15, #0, %[zero], c7, c5,  #6" : : [zero] "r" (0) )

	#define invalidate_data_cache()			asm volatile ("mcr p15, 0, %0, c7, c6,  0\n" \
														  "mcr p15, 0, %0, c7, c10, 4\n" : : "r" (0) : "memory")


	#define clean_data_cache()				asm volatile ("mcr p15, 0, %0, c7, c10, 0\n" \
														  "mcr p15, 0, %0, c7, c10, 4\n" : : "r" (0) : "memory")
#else
	#define isb() asm volatile ("isb" ::: "memory")
	#define dsb() asm volatile ("dsb" ::: "memory")
	#define dmb() asm volatile ("dmb" ::: "memory")

	#define invalidate_instruction_cache()	asm volatile ("mcr p15, #0, %[zero], c7, c5,  #0" : : [zero] "r" (0) : "memory")
	#define flush_prefetch_buffer()			asm volatile ("isb" ::: "memory")
	#define flush_branch_target_cache() 	asm volatile ("mcr p15, #0, %[zero], c7, c5,  #6" : : [zero] "r" (0) : "memory")

	extern void invalidate_data_cache(void) __attribute__ ((optimize (3)));
	extern void clean_data_cache(void) __attribute__ ((optimize (3)));
	extern void invalidate_data_cache_l1_only(void) __attribute__ ((optimize (3)));
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYNCHRONIZE_H_ */
