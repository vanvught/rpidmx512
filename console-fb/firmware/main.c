#include <stdlib.h>
#include <stdio.h>
#include <bcm2835.h>
#include <bcm2835_vc.h>

void c_irq_handler(void) {}
void c_fiq_handler(void) {}

extern void fb_init(void);

extern void cpu_info(void);
extern void mem_info(void);

int notmain(uint32_t boot_dev, uint32_t arm_m_type, uint32_t atags)
{
	fb_init();

    printf("Compiled on %s at %s\n\n", __DATE__, __TIME__);

    uint64_t ts = bcm2835_st_read();

    mem_info();

    printf("\n%ld usec elapsed\n\n", (long int)(bcm2835_st_read() - ts));

	cpu_info();

	printf("\n");

	printf("EMMC Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_EMMC));
	printf("UART Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_UART));
	printf("ARM  Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_ARM));
	printf("CORE Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_CORE));

    printf("\nProgram end\n");

    return 0;
}
