/**
 * @file main.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdbool.h>

#include "bcm2835_vc.h"
#include "arm/arm.h"

#include "hardware.h"
#include "irq_timer.h"
#include "console.h"
#include "wifi.h"

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

static const char *gpfsel_names[] = { "IN  ", "OUT ", "ALT5", "ALT4", "ALT0", "ALT1", "ALT2", "ALT3" };

static volatile unsigned led_state = 0;

void led_handler(const uint32_t clo) {
	const int state = led_state++ & 0x01;

	console_set_cursor(0, 24);
	console_putc((int) '0' + state);
	hardware_led_set(state);

	BCM2835_ST->C3 = clo + (uint32_t) 1000000;
}

void time_out_handler(const uint32_t clo) {
	BCM2835_ST->C3 = clo;
	console_save_color();
	console_set_fg_color(CONSOLE_RED);
	console_puts("\nFailed connecting to ESP8266\n");
	console_restore_color();
}

void handle_rc(int rc) {
	console_save_color();
	if (rc < 0) {
		console_set_fg_color(CONSOLE_RED);
		console_puts("Failed ");
	} else {
		console_set_fg_color(CONSOLE_GREEN);
		console_puts("OK ");
	}
	console_restore_color();
}

void notmain(void) {
	hardware_init();

	printf("%s [%x]\n\n", hardware_board_get_model(), (unsigned int)hardware_board_get_model_id());
#if defined (RPI1)
	printf("kernel.img : ");
#elif defined (RPI2)
	printf("kernel7.img : ");
#elif defined (RPI3)
	printf("kernel8.img : ");
#else
	printf("Unknown kernel : ");
#endif
	printf("Compiled on %s at %s\n\n", __DATE__, __TIME__);

	uint32_t arm_ram = bcm2835_vc_get_memory(BCM2835_VC_TAG_GET_ARM_MEMORY) / 1024 / 1024;	///< MB
	handle_rc(arm_ram);
	printf("ARM RAM : %d MB\n", (int)arm_ram);

	uint32_t vc_ram = bcm2835_vc_get_memory(BCM2835_VC_TAG_GET_VC_MEMORY) / 1024 / 1024;	///< MB
	handle_rc(vc_ram);
	printf("VC RAM  : %d MB\n", (int)vc_ram);

	uint32_t arm_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_ARM);
	handle_rc(arm_clock);
	printf("ARM CLOCK  : %d MHz\n", (int)arm_clock);

	uint32_t vc_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_CORE);
	handle_rc(vc_clock);
	printf("VC CLOCK   : %d MHz\n", (int)vc_clock);

	uint32_t uart_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_UART);
	handle_rc(uart_clock);
	printf("UART CLOCK : %d MHz\n", (int)uart_clock);

	uint8_t mac_address[6];
	handle_rc(bcm2835_vc_get_board_mac_address(mac_address));
	printf("MAC : %.2x%.2x:%.2x%.2x%.2x%.2x\n\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

	unsigned int n, i, val;

	for (n = 0; n < 6; n++) {								// Loop for GPFSEL0 to GPFSEL5
		val = *((unsigned int *) BCM2835_GPIO_BASE + n) & (n < 5 ? 0x3fffffff : 0x00000fff);
		printf("GPFSEL%d ", n);
		for (i = 0; i < 10; i++) {							// Loop for the 9 bit-triplets in each GPFSELn register
			int fsel_num = n * 10 + i;						// The logical FSEL number
			int bit_pos = i * 3;							// The position of the least significant bit in the triplet
			unsigned int triplet = (val >> bit_pos) & 0x7;	// Extract the bit- triplet
			printf("%2d:%s ", fsel_num, gpfsel_names[triplet]);
			if (n == 5 && i == 3)							// break out of loop after FSEL53 (last triplet)
				break;
		}
		printf("\n");
	}

	printf("\n");

	irq_timer_init();
	irq_timer_set(IRQ_TIMER_1, time_out_handler);
	irq_timer_set(IRQ_TIMER_3, led_handler);
	BCM2835_ST->C1 = BCM2835_ST->CLO + (uint32_t) 1000000;
	BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t) 1000000;

	wifi_init("");

	irq_timer_set(IRQ_TIMER_1, NULL);

	printf("ESP8266 information\n");
	printf(" SDK      : %s\n", system_get_sdk_version());
	printf(" Firmware : %s\n\n", wifi_get_firmware_version());

	while (1) {
	}

}

