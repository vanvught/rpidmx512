/**
 * @file mmu.c
 *
 */
/* This code is inspired by:
 *
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
 * https://github.com/rsta2/circle/blob/master/lib/memory.cpp
 * https://github.com/rsta2/circle/blob/master/lib/pagetable.cpp
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

#include <stdint.h>

#include "h3.h"

#include "arm/synchronize.h"

static volatile __attribute__ ((aligned (0x4000))) uint32_t page_table[4096];

// [1:0] 	1 0 section
// [2]		B
// [3]		C
// [4]		XN
// [8:5]	Domain
// [9]		IM
// [11:10]	AP
// [12:14]	TEX
// [15]		APX
// [16]		S
// [17]		NG
// [19:18]	0 0
// [31:20]	base address

/*
 * U-Boot
 * TTBR0 at address: 0x4fff0000
 *  0000 : 0x4fff0000 : 0x00000000 : 00c12 -> bits 1  4  10 11
 *  1024 : 0x4fff1000 : 0x40000000 : 00c1e -> bits 1  2  3  4  10 11
 *  1280 : 0x4fff1400 : 0x50000000 : 00c12 -> bits 1  4  10 11
 *  4095 : 0x4fff3ffc : 0xfff00000 : 00c12 -> bits 1  4  10 11
 */
/*
 * mrc p15, 0, ,, c1, c0, 1 => 0x00006040 -> bits 6  13 14
 * mrc p15, 0, ,, c3, c0, 0 => 0xffffffff
 * mrc p15, 0, ,, c2, c0, 2 => 0x00000000
 * mrc p15, 0, ,, c2, c0, 0 => 0x4fff0059 -> bits 0  3  4  6  16 17 18 19 20 21 22 23 24 25 26 27 30
 */

// Access permissions (AP[11:10])
#define AP_NO_ACCESS		0	// 00
#define AP_SYSTEM_ACCESS	1	// 01
#define AP_USER_RO_ACCESS	2	// 10
#define AP_ALL_ACCESS		3	// 11

// Access permissions extended (APX)
#define APX_RW_ACCESS		0
#define APX_RO_ACCESS		1

// Domains
#define DOMAIN_NO_ACCESS	0	// 00
#define DOMAIN_CLIENT		1	// 01
#define DOMAIN_MANAGER		3	// 11

#define ARM_AUX_CONTROL_SMP					(1 << 6)

#define ARM_TTBR_INNER_NON_CACHEABLE		((0 << 6) | (0 << 0))
#define ARM_TTBR_INNER_WRITE_ALLOCATE		((1 << 6) | (0 << 0))
#define ARM_TTBR_INNER_WRITE_THROUGH		((0 << 6) | (1 << 0))
#define ARM_TTBR_INNER_WRITE_BACK			((1 << 6) | (1 << 0))

#define ARM_TTBR_OUTER_NON_CACHEABLE		(0 << 3)
#define ARM_TTBR_OUTER_WRITE_ALLOCATE		(1 << 3)
#define ARM_TTBR_OUTER_WRITE_THROUGH		(2 << 3)
#define ARM_TTBR_OUTER_WRITE_BACK			(3 << 3)

#define TTBR_MODE	(ARM_TTBR_INNER_WRITE_BACK | ARM_TTBR_OUTER_WRITE_BACK)

#define ARM_CONTROL_MMU						(1 << 0)
#define ARM_CONTROL_STRICT_ALIGNMENT		(1 << 1)
#define ARM_CONTROL_L1_CACHE				(1 << 2)
#define ARM_CONTROL_BRANCH_PREDICTION		(1 << 11)
#define ARM_CONTROL_L1_INSTRUCTION_CACHE 	(1 << 12)

#define MMU_MODE	(  ARM_CONTROL_MMU			        \
			 	 	 | ARM_CONTROL_L1_CACHE			    \
					 | ARM_CONTROL_L1_INSTRUCTION_CACHE	\
					 | ARM_CONTROL_BRANCH_PREDICTION)

#define SECTION	(0b10 << 0)
#define B_BIT	(1 << 2)
#define C_BIT	(1 << 3)
#define XN_BIT	(1 << 4)
#define DOMAIN	(0 << 5)
#define AP		(AP_SYSTEM_ACCESS << 10)
#define S_BIT	(1 << 16)

#define MACRO_SRAM	(        AP | DOMAIN          | C_BIT | B_BIT | SECTION)
#define MACRO_PERI	(        AP | DOMAIN | XN_BIT |                 SECTION)
#define MACRO_DRAM	(        AP | DOMAIN          | C_BIT | B_BIT | SECTION)
#define MACRO_COHE	(S_BIT | AP | DOMAIN | XN_BIT |                 SECTION)
#define MACRO_BROM	(        AP | DOMAIN          | C_BIT | B_BIT | SECTION)

uint32_t *mmu_get_page_table(void) {
	return (uint32_t *) &page_table;
}

void mmu_enable(void) {
	uint32_t entry;
	const uint32_t dram_size = h3_get_dram_size(); // This is already in MEGABYTE

	page_table[0] = (0 << 20) | MACRO_SRAM;

	for (entry = 1; entry < (H3_MEM_DRAM_START / MEGABYTE); entry ++) {
		page_table[entry] = entry << 20 | MACRO_PERI;
	}

	for (; entry < (H3_MEM_DRAM_START / MEGABYTE) + dram_size; entry++) {
		page_table[entry] = entry << 20 | MACRO_DRAM;
#ifdef ARM_ALLOW_MULTI_CORE
		page_table[entry] |= S_BIT;
#endif
	}

	for (; entry < (H3_MEM_BROM_START / MEGABYTE); entry++) {
		page_table[entry] = entry << 20;
	}

	for (; entry < 4096; entry++) {
		page_table[entry] = entry << 20 | MACRO_BROM;
	}

	// Finally set the Coherent region. This is part of the DRAM section
	entry = H3_MEM_COHERENT_REGION / MEGABYTE;
	page_table[entry] = entry << 20 | MACRO_COHE;

	clean_data_cache();
	dmb();

	uint32_t auxctrl;
	asm volatile ("mrc p15, 0, %0, c1, c0,  1" : "=r" (auxctrl));
	auxctrl |= ARM_AUX_CONTROL_SMP;
	asm volatile ("mcr p15, 0, %0, c1, c0,  1" :: "r" (auxctrl));

	// set domain 0 to manager (client does not work)
	asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (DOMAIN_MANAGER << 0));

	// always use TTBR0
	asm volatile ("mcr p15, 0, %0, c2, c0, 2" :: "r" (0));

	// set TTBR0
	asm volatile ("mcr p15, 0, %0, c2, c0, 0" :: "r" (TTBR_MODE | (uint32_t) &page_table));

	isb();

#ifndef ARM_ALLOW_MULTI_CORE
	invalidate_data_cache();
#else
	invalidate_data_cache_l1_only();
#endif

	// required if MMU was previously enabled and not properly reset
	invalidate_instruction_cache();
	flush_branch_target_cache();
	asm volatile ("mcr p15, 0, %0, c8, c7,  0" : : "r" (0)); // invalidate unified TLB
	dsb();
	flush_prefetch_buffer();

	uint32_t control;
	asm volatile ("mrc p15, 0, %0, c1, c0,  0" : "=r" (control));
	control &= ~(0x2);
	control |= MMU_MODE;
	asm volatile ("mcr p15, 0, %0, c1, c0,  0" : : "r" (control) : "memory");
}
