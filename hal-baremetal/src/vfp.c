
#include "arm/synchronize.h"

void vfp_init(void) {
	// Coprocessor Access Control Register
	unsigned nCACR;
	__asm volatile ("mrc p15, 0, %0, c1, c0, 2" : "=r" (nCACR));
	nCACR |= 3 << 20;	// cp10 (single precision)
	nCACR |= 3 << 22;	// cp11 (double precision)
	__asm volatile ("mcr p15, 0, %0, c1, c0, 2" : : "r" (nCACR));
	isb();

#define VFP_FPEXC_EN	(1 << 30)
	__asm volatile ("fmxr fpexc, %0" : : "r" (VFP_FPEXC_EN));

	__asm volatile ("fmxr fpscr, %0" : : "r" (0));
}
