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
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/synchronize.h"
#include "bcm2835.h"
#include "bcm2835_vc.h"

#define MEGABYTE		0x100000

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

// Access permissions (AP[11:10])
#define AP_NO_ACCESS		0	// 00
#define AP_SYSTEM_ACCESS	1	// 01
#define AP_USER_RO_ACCESS	2	// 10
#define AP_ALL_ACCESS		3	// 11

// Access permissions extended (APX)
#define APX_RW_ACCESS		0
#define APX_RO_ACCESS		1

// Domains
#define DOMAIN_NO_ACCESS	0
#define DOMAIN_CLIENT		1
#define DOMAIN_MANAGER		3


//  Auxiliary Control register
#if defined (RPI1)
#define ARM_AUX_CONTROL_CACHE_SIZE	(1 << 6)	// restrict cache size to 16K (no page coloring)
#else
#define ARM_AUX_CONTROL_SMP			(1 << 6)
#endif

#define ARM_TTBR_INNER_NON_CACHEABLE	((0 << 6) | (0 << 0))
#define ARM_TTBR_INNER_WRITE_ALLOCATE	((1 << 6) | (0 << 0))
#define ARM_TTBR_INNER_WRITE_THROUGH	((0 << 6) | (1 << 0))
#define ARM_TTBR_INNER_WRITE_BACK		((1 << 6) | (1 << 0))

#define ARM_TTBR_OUTER_NON_CACHEABLE	(0 << 3)
#define ARM_TTBR_OUTER_WRITE_ALLOCATE	(1 << 3)
#define ARM_TTBR_OUTER_WRITE_THROUGH	(2 << 3)
#define ARM_TTBR_OUTER_WRITE_BACK		(3 << 3)

#define TTBR_MODE	(ARM_TTBR_INNER_WRITE_BACK | ARM_TTBR_OUTER_WRITE_BACK)

#define ARM_CONTROL_MMU						(1 << 0)
#define ARM_CONTROL_STRICT_ALIGNMENT		(1 << 1)
#define ARM_CONTROL_L1_CACHE				(1 << 2)
#define ARM_CONTROL_BRANCH_PREDICTION		(1 << 11)
#define ARM_CONTROL_L1_INSTRUCTION_CACHE 	(1 << 12)

#define MMU_MODE	(  ARM_CONTROL_MMU			\
			 | ARM_CONTROL_L1_CACHE			\
			 | ARM_CONTROL_L1_INSTRUCTION_CACHE	\
			 | ARM_CONTROL_BRANCH_PREDICTION)

static volatile uint32_t arm_ram = 0;

uint32_t mmu_get_arm_ram(void) {
	return arm_ram;
}

const uint32_t *mmu_get_page_table(void) {
	return (uint32_t *) &page_table;
}

void mmu_enable(void) {

	uint32_t entry;
	uint32_t entries;

	arm_ram = bcm2835_vc_get_memory(BCM2835_VC_TAG_GET_ARM_MEMORY) / 1024 / 1024;	///< MB

	for (entry = 0; entry < arm_ram; entry++) {
#ifndef ARM_ALLOW_MULTI_CORE	// S = 0
													///< 31   27   23   19   15   11   7    3
													///<   28   24   20   16   12    8    4    0
		page_table[entry] = entry << 20 | 0x0040E;	///< 0000 0000 0000 0000 0000 0100 0000 1110
#else							// S = 1, needed for spin locks
													///< 31   27   23   19   15   11   7    3
													///<   28   24   20   16   12    8    4    0
	    page_table[entry] = entry << 20 | 0x1040E;	///< 0000 0000 0000 0001 0000 0100 0000 1110
#endif
	}

	for (; entry < 1024; entry++) {
	    // shared device, never execute
													///< 31   27   23   19   15   11   7    3
													///<   28   24   20   16   12    8    4    0
	    page_table[entry] = entry << 20 | 0x10416;	///< 0000 0000 0000 0001 0000 0100 0001 0110
	}

	// 1 MB mailboxes at 0x40000000 Multi-core
    // shared device, never execute
													///< 31   27   23   19   15   11   7    3
													///<   28   24   20   16   12    8    4    0
    page_table[entry] = entry << 20 | 0x10416;		///< 0000 0000 0000 0001 0000 0100 0001 0110
	++entry;

	// unused rest of address space)
	for (; entry < 4096; entry++) {
	    page_table[entry] = 0;
	}

	//B = 0, C = 0, TEX = 0, S = 0
	entry = BCM2835_PERI_BASE / MEGABYTE;
	entries = entry + (0x1000000 / MEGABYTE);

	for (; entry < entries; entry++) {
													///< 31   27   23   19   15   11   7    3
													///<   28   24   20   16   12    8    4    0
		page_table[entry] = entry << 20 | 0x00412;	///< 0000 0000 0000 0000 0000 0100 0001 0010
	}

	//B = 0, C = 0, TEX = 0, S = 1
	entry = MEM_COHERENT_REGION / MEGABYTE ;
													///< 31   27   23   19   15   11   7    3
													///<   28   24   20   16   12    8    4    0
	page_table[entry] = entry << 20 | 0x10412;		///< 0000 0000 0000 0001 0000 0100 0001 0010

	clean_data_cache();
	dmb();

	uint32_t auxctrl;
	asm volatile ("mrc p15, 0, %0, c1, c0,  1" : "=r" (auxctrl));
#if defined (RPI1)
	auxctrl |= ARM_AUX_CONTROL_CACHE_SIZE;
#else
	auxctrl |= ARM_AUX_CONTROL_SMP;
#endif
	asm volatile ("mcr p15, 0, %0, c1, c0,  1" :: "r" (auxctrl));

	// set domain 0 to client
	asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (DOMAIN_CLIENT << 0));

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

	// enable MMU
	uint32_t control;
	asm volatile ("mrc p15, 0, %0, c1, c0,  0" : "=r" (control));
	control |= MMU_MODE;
	asm volatile ("mcr p15, 0, %0, c1, c0,  0" : : "r" (control) : "memory");
}

