#include <bcm2835.h>
#include <bcm2835_pl011.h>

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

extern void __enable_irq(void);

static void pl011_sendstr(const char *s) {
	char c;
	while((c = *s++)) bcm2835_pl011_send(c);
}

static void irq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
	BCM2835_IRQ->IRQ_ENABLE2 = 1 << 25;
}

volatile unsigned int rxhead;
volatile unsigned int rxtail;

#define RXBUFMASK 0xFFF
volatile unsigned char rxbuffer[RXBUFMASK+1];

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	uint32_t mis = BCM2835_PL011 -> MIS;

	if (mis & PL011_MIS_RXMIS ) {
		do {
			uint32_t data = BCM2835_PL011 ->DR;
			rxbuffer[rxhead] = data & 0xFF;
			rxhead = (rxhead + 1) & RXBUFMASK;
		} while (!(BCM2835_PL011 ->FR & PL011_FR_RXFE));
	}

}

int notmain(unsigned int earlypc) {

	bcm2835_pl011_begin();

	pl011_sendstr("Init IRQ\r\n");
	irq_init();

	pl011_sendstr("Enable IRQ\r\n");
	__enable_irq();

	rxtail = rxhead;

	for (;;) {
		while (rxtail != rxhead) {
			bcm2835_pl011_send(rxbuffer[rxtail]);
			rxtail = (rxtail + 1) & RXBUFMASK;
		}
	}

	return (0);
}
