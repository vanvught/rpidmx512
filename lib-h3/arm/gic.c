/**
 * @file gic.c
 *
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
#include <stdbool.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "arm/synchronize.h"
#include "arm/gic.h"

static uint16_t max_interrupts;
static uint8_t impemented_cpu_interfaces;

#define ICENABLE_N	(sizeof(H3_GIC_DIST->ICENABLE) / sizeof(H3_GIC_DIST->ICENABLE[0]))
#define ICPEND_N	(sizeof(H3_GIC_DIST->ICPEND) / sizeof(H3_GIC_DIST->ICPEND[0]))
#define ICACTIVE_N	(sizeof(H3_GIC_DIST->ICACTIVE) / sizeof(H3_GIC_DIST->ICACTIVE[0]))
#define IPRIORITY_N	(sizeof(H3_GIC_DIST->IPRIORITY) / sizeof(H3_GIC_DIST->IPRIORITY[0]))
#define ITARGETS_N	(sizeof(H3_GIC_DIST->ITARGETS) / sizeof(H3_GIC_DIST->ITARGETS[0]))
#define ICFG_N		(sizeof(H3_GIC_DIST->ICFG) / sizeof(H3_GIC_DIST->ICFG[0]))

#define CTL_ENABLE_GRP0_MASK (1 << 0)		///< Bit 0, Global enable for forwarding pending Group 0 interrupts from the Distributor to the CPU interfaces
	#define CTL_GRP0_ENABLE		(1 << 0)	///<
	#define CTL_GRP0_DISABLE	(0 << 0)	///<

#define CTL_ENABLE_GRP1_MASK (1 << 1)		///< Bit 1, Global enable for forwarding pending Group 1 interrupts from the Distributor to the CPU interfaces
	#define CTL_GRP1_ENABLE		(1 << 1)	///<
	#define CTL_GRP1_DISABLE	(0 << 1)	///<

#define GICC_CTL_ENABLE_GRP0	(1 << 0)
#define GICC_CTL_ENABLE_GRP1	(1 << 1)
#define GICC_CTL_ENABLE_FIQ		(1 << 3)

void gic_init(void) {
	unsigned i;

	max_interrupts = (uint16_t) ((H3_GIC_DIST->TYPE & 0x1F) + 1) * 32;
	impemented_cpu_interfaces = (uint8_t) 1 + (uint8_t) ((H3_GIC_DIST->TYPE >> 5) & 0x7);

	/* Initialize Distributor */

	H3_GIC_DIST->CTL = 0;

	for (i = 0; i < ICENABLE_N; i++) {
		H3_GIC_DIST->ICENABLE[i] = 0xFFFFFFFF;	// Writing 1 to a Clear-enable bit disables forwarding
	}

	for (i = 0; i < ICPEND_N; i++) {
		H3_GIC_DIST->ICPEND[i] = 0xFFFFFFFF;	// Writing 1 to a Clear-pending bit clears the pending states
	}

	for (i = 0; i < ICACTIVE_N; i++) {
		H3_GIC_DIST->ICACTIVE[i] = 0xFFFFFFFF;	// Writing 1 to a Clear-active bit Deactivates the corresponding interrupt
	}

	for (i = 0; i < IPRIORITY_N; i++) {
		H3_GIC_DIST->IPRIORITY[i] = 0x0;		// TODO What is this?
	}

	for (i = 0; i < ITARGETS_N; i++) {
		H3_GIC_DIST->ITARGETS[i] = 0x01010101; // TODO This needs be changed for multi-core support
	}

	for (i = 0; i < ICFG_N; i++) {
		H3_GIC_DIST->ICFG[i] = 0x0;				// 0 => Corresponding interrupt is level-sensitive.
	}

	H3_GIC_DIST->CTL = CTL_GRP0_ENABLE | CTL_GRP1_ENABLE;	// Group 1 -> IRQ, Group 0 -> FIQ

	/* Initialize CPU Interface */
	H3_GIC_CPUIF->PM = 0xFFFFFFFF;

	H3_GIC_CPUIF->CTL = GICC_CTL_ENABLE_GRP0 | GICC_CTL_ENABLE_GRP1 | GICC_CTL_ENABLE_FIQ;
}

static void int_config(H3_IRQn_TypeDef n, GIC_CORE_TypeDef cpu, GIC_GROUP_TypeDef group) {
#ifndef NDEBUG
	printf("int_config(H3_IRQn_TypeDef %d,cpu %d, GIC_GROUP_TypeDef %d)\n", n, cpu, group);
#endif

	clean_data_cache();
	dmb();

	const uint32_t index = n / 32;
	uint32_t mask = 1 << (n % 32);

	H3_GIC_DIST->ISENABLE[index] = mask;

#ifndef NDEBUG
	printf("H3_GIC_DIST->ISENABLE[%d] = %p [%p]\n", index, mask, H3_GIC_DIST->ISENABLE[index]);
#endif

	mask = group << (n % 32);

	H3_GIC_DIST->IGROUP[index] |= mask;

	isb();

#ifndef NDEBUG
	printf("H3_GIC_DIST->IGROUP[%d] = %p [%p]\n", index, mask, H3_GIC_DIST->IGROUP[index]);
#endif
}

void gic_irq_config(H3_IRQn_TypeDef n, GIC_CORE_TypeDef cpu) {
	int_config(n, cpu, GIC_GROUP1);
}

void gic_fiq_config(H3_IRQn_TypeDef n, GIC_CORE_TypeDef cpu) {
	int_config(n, cpu, GIC_GROUP0);
}

void gic_init_dump(void) {
#ifndef NDEBUG
	unsigned i;

	printf("max_interrupts=%d, impemented_cpu_interfaces=%d\n", max_interrupts, impemented_cpu_interfaces);

	printf("[%p] CTL = %p\n", &H3_GIC_DIST->CTL, H3_GIC_DIST->CTL);
	printf("[%p] TYPE = %p\n", &H3_GIC_DIST->TYPE, H3_GIC_DIST->TYPE);
	printf("[%p] IID = %p\n", &H3_GIC_DIST->IID, H3_GIC_DIST->IID);

	printf("[%p] ICENABLE, ICENABLE_N=%d, IMPL_N=%d\n", &H3_GIC_DIST->ICENABLE, ICENABLE_N, max_interrupts/32);
	for (i = 0; i < max_interrupts/32; i++) {
		printf("\t[%p] ICENABLE[%d] = %p\n", &H3_GIC_DIST->ICENABLE[i], i, H3_GIC_DIST->ICENABLE[i]);
	}

	printf("[%p] ICPEND, ICPEND_N=%d, IMPL_N=%d\n", &H3_GIC_DIST->ICPEND, ICPEND_N, max_interrupts/32);
	for (i = 0; i <  max_interrupts/32; i++) {
		printf("\t[%p] ICPEND[%d] = %p\n", &H3_GIC_DIST->ICPEND[i], i, H3_GIC_DIST->ICPEND[i]);
	}

	printf("[%p] ICACTIVE, ICACTIVE_N=%d, IMPL_N=%d\n", &H3_GIC_DIST->ICACTIVE, ICACTIVE_N, max_interrupts/32);
	for (i = 0; i < max_interrupts/32; i++) {
		printf("\t[%p] ICACTIVE[%d] = %p\n", &H3_GIC_DIST->ICACTIVE[i], i, H3_GIC_DIST->ICACTIVE[i]);
	}

	printf("[%p] IPRIORITY, IPRIORITY_N=%d, IMPL_N=%d\n", &H3_GIC_DIST->IPRIORITY, IPRIORITY_N, max_interrupts/4);
	for (i = 0; i < max_interrupts/4; i++) {
		printf("\t[%p] IPRIORITY[%d] = %p\n", &H3_GIC_DIST->IPRIORITY[i], i, H3_GIC_DIST->IPRIORITY[i]);
	}

	printf("[%p] ICFG, ICFG_N=%d, IMPL_N=%d\n", &H3_GIC_DIST->ICFG, ICFG_N, max_interrupts/16);
	for (i = 0; i < max_interrupts/16; i++) {
		printf("\t[%p] ICFG[%d] = %p\n", &H3_GIC_DIST->ICFG[i], i, H3_GIC_DIST->ICFG[i]);
	}
#endif	
}

void gic_int_dump(H3_IRQn_TypeDef n) {
#ifndef NDEBUG
	uint32_t index = n / 32;
	uint32_t mask = 1 << (n % 32);

	GIC_GROUP_TypeDef group = (GIC_GROUP_TypeDef) ((H3_GIC_DIST->IGROUP[index] & mask) == mask);
	bool is_enabled = (H3_GIC_DIST->ISENABLE[index] & mask) == mask;
	bool is_pending = (H3_GIC_DIST->ISPEND[index] & mask) == mask;

	printf("H3_IRQn=%d (index=%d,mask=%p), group=%d, enabled=%d, pending=%d\n", n, index, mask, group, is_enabled, is_pending);
#endif
}
