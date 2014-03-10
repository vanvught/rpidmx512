#include <stdio.h>
#include <bcm2835.h>
#include <bcm2835_vc.h>
#include <hardware.h>

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

extern void fb_init(void);

uint8_t dmx_data[512];

#define ANALYZER_CH1	RPI_V2_GPIO_P1_23     // CLK
#define ANALYZER_CH2   RPI_V2_GPIO_P1_21     // MISO
#define ANALYZER_CH3   RPI_V2_GPIO_P1_19     // MOSI
#define ANALYZER_CH4   RPI_V2_GPIO_P1_24     // CE0

// -------------------------------------------------------------------------------------- //

static void bcm2835_pl011_dmx512_init(void) {
	// Set UART clock rate to 4000000 (4MHz)
	bcm2835_vc_set_clock_rate(BCM2835_MAILBOX_CLOCK_ID_UART, 4000000);
	//
	BCM2835_PL011->CR	= 0;										// Disable everything
	//
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0);	// PL011_TXD
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0);	// PL011_RXD
	//
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	//
	while (BCM2835_PL011 ->FR & PL011_FR_BUSY ) {}					// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	//
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;							// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR 	= 0x7FF;									// Clear all interrupt status
	BCM2835_PL011->IBRD = 1;										// UART Clock
	BCM2835_PL011->FBRD = 0;										// 4000000 (4MHz)
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 ;		// Set 8, N, 2, FIFO disabled
	BCM2835_PL011->CR 	= 0x301;									// Enable UART
}

// -------------------------------------------------------------------------------------- //

static void fiq_init(void) {
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
    BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_VC_UART;
}

// State of receiving DMX Bytes
typedef enum {
  IDLE, BREAK, DATA
} _dmx_receive_state;

uint8_t dmx_receive_state = IDLE;
uint16_t dmx_data_index = 0;

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	bcm2835_gpio_set(ANALYZER_CH1);

	if (BCM2835_PL011 ->DR & PL011_DR_BE ) {
		bcm2835_gpio_set(ANALYZER_CH2); // BREAK
		bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
		bcm2835_gpio_clr(ANALYZER_CH4);	// IDLE

		dmx_receive_state = BREAK;
	}  else if (dmx_receive_state == BREAK) {
		if ((BCM2835_PL011 ->DR & 0xFF) == 0) {
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_set(ANALYZER_CH3);	// DATA
			bcm2835_gpio_clr(ANALYZER_CH4); // IDLE

			dmx_receive_state = DATA;
			dmx_data_index = 0;
		} else {
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
	} else if (dmx_receive_state == DATA) {
		dmx_data[dmx_data_index] = (BCM2835_PL011 ->DR & 0xFF);
		dmx_data_index++;
		if (dmx_data_index >= 512) {
			bcm2835_gpio_clr(ANALYZER_CH2); // BREAK
			bcm2835_gpio_clr(ANALYZER_CH3);	// DATA
			bcm2835_gpio_set(ANALYZER_CH4); // IDLE

			dmx_receive_state = IDLE;
		}
	}

	bcm2835_gpio_clr(ANALYZER_CH1);
}

// -------------------------------------------------------------------------------------- //

void task_fb(void) {
#if 0
	printf("%X %X %X %X %X %X %X %X\n", dmx_data[0], dmx_data[1], dmx_data[2],
			dmx_data[3], dmx_data[4], dmx_data[5], dmx_data[6], dmx_data[7]);
	printf("%X %X %X %X %X %X %X %X\n\n", dmx_data[8], dmx_data[9],
			dmx_data[10], dmx_data[11], dmx_data[12], dmx_data[13],
			dmx_data[14], dmx_data[15]);
#else
	printf("%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X\n", dmx_data[0],
			dmx_data[1], dmx_data[2], dmx_data[3], dmx_data[4], dmx_data[5],
			dmx_data[6], dmx_data[7], dmx_data[8], dmx_data[9], dmx_data[10],
			dmx_data[11], dmx_data[12], dmx_data[13], dmx_data[14],
			dmx_data[15]);
#endif
}

void task_led(void) {
	static unsigned char led_counter = 0;
	led_set(led_counter++ & 0x01);
}

typedef struct _event {
	uint64_t period;
	void (*f)(void);
} event;

const event events[] = {
		{ 1000000, task_fb},
		{ 500000, task_led}
};

uint64_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

void events_init() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += st_read_now;
	}
}

static inline void events_check() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (st_read_now > events_elapsed_time[i] + events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
		}
	}
}

int notmain(unsigned int earlypc) {

	int i;

	for(i = 0; i < sizeof(dmx_data); i++) {
		dmx_data[i] = 0;
	}

	__disable_fiq();

	fb_init();

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);

	led_init();
	led_set(1);

	bcm2835_gpio_fsel(ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_clr(ANALYZER_CH1); // IRQ
	bcm2835_gpio_clr(ANALYZER_CH2);	// BREAK
	bcm2835_gpio_clr(ANALYZER_CH3); // DATA
	bcm2835_gpio_set(ANALYZER_CH4);	// IDLE

	bcm2835_pl011_dmx512_init();

	fiq_init();
	__enable_fiq();

	events_init();

	for (;;) {
		events_check();
	}

	return (0);
}
