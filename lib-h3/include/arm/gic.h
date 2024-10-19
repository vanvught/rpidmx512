/**
 * @file gic.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GIC_H_
#define GIC_H_

#include "h3.h"

typedef enum {
	GIC_GROUP0 = 0,
	GIC_GROUP1 = 1
} GIC_GROUP_TypeDef;

typedef enum {
	GIC_CORE0 = 1,
	GIC_CORE1 = 2,
	GIC_CORE2 = 3,
	GIC_CORE3 = 4
} GIC_CORE_TypeDef;

typedef struct {
	__IO uint32_t CTL;				///< 0x000 Distributor Control Register
	__I uint32_t TYPE;				///< 0x004 Interrupt Controller Type Register
	__I uint32_t IID;				///< 0x008 Distributor Implementer Identification Register
	__IO uint32_t RES1[5];			///< 0x00C-0x01C Reserved
	__IO uint32_t IMP1[8];			///< 0x020-0x03C IMPLEMENTATION DEFINED registers
	__IO uint32_t RES2[16];			///< 0x040-0x07C Reserved
	__IO uint32_t IGROUP[16];		///< 0x080 Interrupt Group Registers
	__IO uint32_t RES3[16];			///< 0x080-0x0FC Reserved
	__IO uint32_t ISENABLE[16];		///< 0x100 Interrupt Set-Enable Registers
	__IO uint32_t RES4[16];			///< Reserved
	__IO uint32_t ICENABLE[16];		///< 0x180 Interrupt Clear-Enable Registers
	__IO uint32_t RES5[16];			///< Reserved
	__IO uint32_t ISPEND[16];		///< 0x200 Interrupt Set-Pending Registers
	__IO uint32_t RES6[16];			///< Reserved
	__IO uint32_t ICPEND[16];		///< 0x280 Interrupt Clear-Pending Registers
	__IO uint32_t RES7[16];			///< Reserved
	__IO uint32_t ISACTIVE[16];		///< 0x300 GICv2 Interrupt Set-Active Registers
	__IO uint32_t RES8[16];			///< Reserved
	__IO uint32_t ICACTIVE[16];		///< 0x380 Interrupt Clear-Active Registers
	__IO uint32_t RES9[16];			///< Reserved
	__IO uint32_t IPRIORITY[128];	///< 0x400 Interrupt Priority Registers
	__IO uint32_t RES10[128];		///< Reserved
	__IO uint32_t ITARGETS[128];	///< 0x800 Interrupt Processor Targets Registers
	__IO uint32_t RES11[128];		///< Reserved
	__IO uint32_t ICFG[32];			///< 0xC00 Interrupt Configuration Registers
	__IO uint32_t RES12[96];		///< Reserved
	__IO uint32_t NSAC[64];			///< 0xE00 Non-secure Access Control Registers, optional
	__O uint32_t SGI;				///< 0xF00 Software Generated Interrupt Register
	__IO uint32_t RES13[3];			///< Reserved
	__IO uint32_t CPENDSGI[4];		///< 0xF10 SGI Clear-Pending Registers
	__IO uint32_t SPENDSGI[4];		///< 0xF20 SGI Set-Pending Registers
	__IO uint32_t RES14[40];		///< Reserved
	__I uint32_t PID4;				///< Peripheral ID 4 Register
	__I uint32_t PID5;				///< Peripheral ID 5 Register
	__I uint32_t PID6;				///< Peripheral ID 6 Register
	__I uint32_t PID7;				///< Peripheral ID 7 Register
	__I uint32_t PID0;				///< Peripheral ID 0 Register
	__I uint32_t PID1;				///< Peripheral ID 1 Register
	__I uint32_t PID2;				///< Peripheral ID 2 Register [7:4] 0x2 for GICv2
	__I uint32_t PID3;				///< Peripheral ID 3 Register
	__I uint32_t ICCID[4];			///< 0xFF0 Component ID Registers -> 0x00, 0xF0, 0x05, 0xB1
	__I uint32_t ICPID[2];			///< 0xFF0 Peripheral ID Registers -> 0x90, 0xB4 -> ARM GICv2
} GIC_DIST_TypeDef;

typedef struct {
	__IO uint32_t CTL;	///< 0x00 CPU Interface Control Register
	__IO uint32_t PM;	///< 0x04 Interrupt Priority Mask Register
	__IO uint32_t BP;	///< 0x08 Binary Point Register
	__I uint32_t IA;	///< 0x0C Interrupt Acknowledge Register
	__O uint32_t EOI;	///< 0x10 End of Interrupt Register
	__I uint32_t RP;	///< 0x14 Running Priority Register
	__I uint32_t HPPI;	///< 0x18 Highest Priority Pending Interrupt Register
	__IO uint32_t ABP;	///< 0x1C Aliased Binary Point Register
	__I uint32_t AIA;	///< 0x20 Aliased Interrupt Acknowledge Register
	__O uint32_t AEOI;	///< 0x24 Aliased End of Interrupt Register
} GIC_CPUIF_TypeDef;

#ifdef __cplusplus
# define _CAST(x)	reinterpret_cast<x>
#else
# define _CAST(x)	(x)
#endif

#define H3_GIC_DIST		(_CAST(GIC_DIST_TypeDef *)(H3_GIC_DIST_BASE))
#define H3_GIC_CPUIF	(_CAST(GIC_CPUIF_TypeDef *)(H3_GIC_CPUIF_BASE))

void gic_init();

void gic_irq_config(H3_IRQn_TypeDef, GIC_CORE_TypeDef);
void gic_fiq_config(H3_IRQn_TypeDef, GIC_CORE_TypeDef);

void gic_init_dump();
void gic_int_dump(H3_IRQn_TypeDef);

template<H3_IRQn_TypeDef IRQn>
inline void gic_unpend() {
	constexpr auto nIndex = IRQn / 32;
	constexpr auto nMask = 1U << (IRQn % 32);
	GICDistributor->ICPENDR[nIndex] = nMask;
}

inline uint32_t gic_get_priority(H3_IRQn_TypeDef IRQn) {
	return (H3_GIC_DIST->IPRIORITY[IRQn / 4U] >> ((IRQn % 4U) * 8U)) & 0xFFUL;
}

inline void gic_set_priority(H3_IRQn_TypeDef IRQn, uint32_t priority) {
	const uint32_t mask = H3_GIC_DIST->IPRIORITY[IRQn / 4U] & ~(0xFFUL << ((IRQn % 4U) * 8U));
	H3_GIC_DIST->IPRIORITY[IRQn / 4U] = mask | ((priority & 0xFFUL) << ((IRQn % 4U) * 8U));
}

/**
 * https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core_A/Source/irq_ctrl_gic.c#L239
 */
inline uint32_t gic_get_active_fiq() {
	/* Dummy read to avoid GIC 390 errata 801120 */
	(void) H3_GIC_CPUIF->HPPI;

	const uint32_t fiqn = H3_GIC_CPUIF->IA;
	__DSB();

	/* Workaround GIC 390 errata 733075 (GIC-390_Errata_Notice_v6.pdf, 09-Jul-2014)  */
	/* The following workaround code is for a single-core system.  It would be       */
	/* different in a multi-core system.                                             */
	/* If the ID is 0 or 0x3FE or 0x3FF, then the GIC CPU interface may be locked-up */
	/* so unlock it, otherwise service the interrupt as normal.                      */
	/* Special IDs 1020=0x3FC and 1021=0x3FD are reserved values in GICv1 and GICv2  */
	/* so will not occur here.                                                       */

	if ((fiqn == 0) || (fiqn >= 0x3FE)) {
		/* Unlock the CPU interface with a dummy write to Interrupt Priority Register */
		const uint32_t prio = gic_get_priority(static_cast<H3_IRQn_TypeDef>(0));
		gic_set_priority(static_cast<H3_IRQn_TypeDef>(0), prio);

		__DSB();
		/* End of Workaround GIC 390 errata 733075 */
	}

	return fiqn;
}

#endif /* GIC_H_ */
