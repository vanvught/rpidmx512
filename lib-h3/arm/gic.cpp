/**
 * @file gic.c
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
# include <stdio.h>
#endif

#include "h3.h"
#include "arm/gic.h"

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

extern "C" void clean_data_cache() __attribute__ ((optimize (3)));

void __attribute__((cold)) gic_init() {
	GIC_Enable();
	H3_GIC_DIST->CTL = CTL_GRP0_ENABLE | CTL_GRP1_ENABLE;	// Group 1 -> IRQ, Group 0 -> FIQ
	H3_GIC_CPUIF->PM = 0xFFFFFFFF;
	GICInterface->CTLR = GICC_CTL_ENABLE_GRP0 | GICC_CTL_ENABLE_GRP1 | GICC_CTL_ENABLE_FIQ;
}

static void int_config(H3_IRQn_TypeDef nIRQ, [[maybe_unused]] GIC_CORE_TypeDef cpu, GIC_GROUP_TypeDef group) {
#ifndef NDEBUG
	printf("int_config(H3_IRQn_TypeDef %d,cpu %d, GIC_GROUP_TypeDef %d)\n", nIRQ, cpu, group);
#endif

	clean_data_cache();
	__DMB();

	GIC_EnableIRQ(nIRQ);
	GIC_SetGroup(nIRQ, static_cast<uint32_t>(group));

	__ISB();

#ifndef NDEBUG
	const auto index = nIRQ / 32;
	auto mask = 1U << (nIRQ % 32);
	printf("H3_GIC_DIST->ISENABLE[%d] = %p [%p]\n", index, mask, H3_GIC_DIST->ISENABLE[index]);
	mask = static_cast<uint32_t>(group << (nIRQ % 32));
	printf("H3_GIC_DIST->IGROUP[%d] = %p [%p]\n", index, mask, H3_GIC_DIST->IGROUP[index]);
#endif
}

void gic_irq_config(H3_IRQn_TypeDef nIRQ, GIC_CORE_TypeDef nCPU) {
	int_config(nIRQ, nCPU, GIC_GROUP1);
	gic_int_dump(nIRQ);
}

void gic_fiq_config(H3_IRQn_TypeDef nIRQ, GIC_CORE_TypeDef nCPU) {
	int_config(nIRQ, nCPU, GIC_GROUP0);
}

void gic_init_dump() {
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

void gic_int_dump(__attribute__((unused)) H3_IRQn_TypeDef n) {
#ifndef NDEBUG
	uint32_t index = n / 32;
	uint32_t mask = 1U << (n % 32);

	GIC_GROUP_TypeDef group = static_cast<GIC_GROUP_TypeDef>((H3_GIC_DIST->IGROUP[index] & mask) == mask);
	bool is_enabled = (H3_GIC_DIST->ISENABLE[index] & mask) == mask;
	bool is_pending = (H3_GIC_DIST->ISPEND[index] & mask) == mask;

	printf("H3_IRQn=%d (index=%d,mask=%p), group=%d, enabled=%d, pending=%d\n", n, index, mask, group, is_enabled, is_pending);
#endif
}
