#include <stdio.h>

unsigned int read_cpuid_id(void) {
	unsigned int __val;
	asm volatile ("mrc p15, #0, %0, c0, c0, #5" : "=r" (__val) : : "cc");
	return __val;
}

#ifdef INCLUDE_CPU_INFO

// The following is derived from:
// https://github.com/raspberrypi/linux/blob/b7c5c10f93a4f48609c3e8decaa37ab5aa04d830/arch/arm/include/asm/system_info.h

#define CPU_ARCH_UNKNOWN 	0
#define CPU_ARCH_ARMv3 		1
#define CPU_ARCH_ARMv4 		2
#define CPU_ARCH_ARMv4T 	3
#define CPU_ARCH_ARMv5 		4
#define CPU_ARCH_ARMv5T	 	5
#define CPU_ARCH_ARMv5TE 	6
#define CPU_ARCH_ARMv5TEJ 	7
#define CPU_ARCH_ARMv6 		8
#define CPU_ARCH_ARMv7 		9

// The following is derived from:
// https://github.com/raspberrypi/linux/blob/b7c5c10f93a4f48609c3e8decaa37ab5aa04d830/arch/arm/kernel/setup.c

static const char *proc_arch[] = {
"undefined/unknown",
"3",
"4",
"4T",
"5",
"5T",
"5TE",
"5TEJ",
"6TEJ",
"7",
"?(11)",
"?(12)",
"?(13)",
"?(14)",
"?(15)",
"?(16)",
"?(17)",
};

static int __get_cpu_architecture(void) {
	int cpu_arch;

	if ((read_cpuid_id() & 0x0008f000) == 0) {
		cpu_arch = CPU_ARCH_UNKNOWN;
	} else if ((read_cpuid_id() & 0x0008f000) == 0x00007000) {
		cpu_arch = (read_cpuid_id() & (1 << 23)) ? CPU_ARCH_ARMv4T : CPU_ARCH_ARMv3;
	} else if ((read_cpuid_id() & 0x00080000) == 0x00000000) {
		cpu_arch = (read_cpuid_id() >> 16) & 7;
		if (cpu_arch) cpu_arch += CPU_ARCH_ARMv3;
	} else if ((read_cpuid_id() & 0x000f0000) == 0x000f0000) {
		unsigned int mmfr0;
		/* Revised CPUID format. Read the Memory Model Feature
		 * Register 0 and check for VMSAv7 or PMSAv7 */
		asm("mrc p15, 0, %0, c0, c1, 4"	: "=r" (mmfr0));
		if ((mmfr0 & 0x0000000f) >= 0x00000003 	|| (mmfr0 & 0x000000f0) >= 0x00000030)
			cpu_arch = CPU_ARCH_ARMv7;
		else if ((mmfr0 & 0x0000000f) == 0x00000002 || (mmfr0 & 0x000000f0) == 0x00000020)
			cpu_arch = CPU_ARCH_ARMv6;
		else
			cpu_arch = CPU_ARCH_UNKNOWN;
	} else
		cpu_arch = CPU_ARCH_UNKNOWN;

	return cpu_arch;
}

void cpu_info(void) {
	printf("CPU identifier\t: 0x%08x\n", read_cpuid_id());
	printf("CPU implementer\t: 0x%02x\n", read_cpuid_id() >> 24);
	printf("CPU architecture: %s\n", proc_arch[__get_cpu_architecture()]);
	printf("CPU revision\t: %d\n", read_cpuid_id() & 15);
	if ((read_cpuid_id() & 0x0008f000) == 0x00000000) {
		/* pre-ARM7 */
		printf("CPU part\t: %07x\n", read_cpuid_id() >> 4);
	} else {
		if ((read_cpuid_id() & 0x0008f000) == 0x00007000) {
			/* ARM7 */
			printf("CPU variant\t: 0x%02x\n", (read_cpuid_id() >> 16) & 127);
		} else {
			/* post-ARM7 */
			printf("CPU variant\t: 0x%x\n", (read_cpuid_id() >> 20) & 15);
		}
		printf("CPU part\t: 0x%03x\n", (read_cpuid_id() >> 4) & 0xfff);
	}
}

#endif
