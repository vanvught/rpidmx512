#include <stdlib.h>
#include <bcm2835.h>

#include "stdio.h"
#include "console.h"

void c_irq_handler(void) {}
void c_fiq_handler(void) {}

extern void fb_init(void);

extern void cpu_info(void);
extern void mem_info(void);

int notmain(uint32_t boot_dev, uint32_t arm_m_type, uint32_t atags)
{
	fb_init();

    printf("Hello World!\n");
    printf("Compiled on %s at %s\n", __DATE__, __TIME__);

    int ch = 32;
    for (ch = 32; ch < 127; ch++)
    	putc(ch, stdout);
    printf("\nputc\n");

    uint64_t ts = bcm2835_st_read();

    mem_info();

    printf("\n%ld usec elapsed\n\n", (long int)(bcm2835_st_read() - ts));

	cpu_info();

    printf("Program end\n");

    return 0;
}
