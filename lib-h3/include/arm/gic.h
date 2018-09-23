/**
 * @file gic.h
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

#define H3_GIC_DIST		((GIC_DIST_TypeDef *) H3_GIC_DIST_BASE)
#define H3_GIC_CPUIF	((GIC_CPUIF_TypeDef *) H3_GIC_CPUIF_BASE)

#ifdef __cplusplus
extern "C" {
#endif

extern void gic_init(void);

extern void gic_irq_config(H3_IRQn_TypeDef n, GIC_CORE_TypeDef cpu);
extern void gic_fiq_config(H3_IRQn_TypeDef n, GIC_CORE_TypeDef cpu);

extern void gic_init_dump(void);
extern void gic_int_dump(H3_IRQn_TypeDef n);

inline static void gic_unpend(H3_IRQn_TypeDef irq) {
	uint32_t index = irq / 32;
	uint32_t mask = 1 << (irq % 32);

	H3_GIC_DIST->ICPEND[index] = mask;
}

#ifdef __cplusplus
}
#endif


#endif /* GIC_H_ */
