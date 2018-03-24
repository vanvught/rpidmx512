/**
 * @file main.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/arm.h"
#include "arm/irq_timer.h"
#include "bcm2835_vc.h"
#ifndef NDEBUG
 #include "bcm2837_gpio_virt.h"
#endif

#include "hardware.h"
#include "console.h"

#include "wifi.h"

#include "lcd.h"
#include "oled.h"

#include "software_version.h"

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

static const char *gpfsel_names[] = { "IN", "OUT ", "ALT5", "ALT4", "ALT0", "ALT1", "ALT2", "ALT3" };

static volatile unsigned led_state = 0;

static void led_handler(const uint32_t clo) {
	const unsigned state = led_state++ & (unsigned) 0x01;

	console_set_cursor(1, 26);
	(void) console_putc((int) '0' + state);
	hardware_led_set((int) state);

	BCM2835_ST->C3 = clo + (uint32_t) 1000000;
}

static void handle_rc(const int32_t rc) {
	console_save_color();
	if (rc < 0) {
		console_set_fg_color(CONSOLE_RED);
		(void) console_puts("Failed ");
	} else {
		console_set_fg_color(CONSOLE_GREEN);
		(void) console_puts("OK ");
	}
	console_restore_color();
}

void notmain(void) {
	int32_t arm_ram;
	int32_t vc_ram;
	int32_t arm_clock;
	int32_t vc_clock;
	int32_t uart_clock;
	uint8_t mac_address[6];
	uint32_t n, i, val;
	oled_info_t oled_info = { OLED_128x32_I2C_DEFAULT };

	printf("[V%s] %s [%x] SoC: %s\n\n", SOFTWARE_VERSION, hardware_board_get_model(), (unsigned int) hardware_board_get_model_id(), hardware_board_get_soc());
#if defined (RPI1)
	(void) console_puts("kernel.img : ");
#elif defined (RPI2)
	(void) console_puts("kernel7.img : ");
#elif defined (RPI3)
	(void) console_puts("kernel8.img : ");
#else
	(void) console_puts("Unknown kernel : ");
#endif
	printf("Compiled on %s at %s\n\n", __DATE__, __TIME__);

	arm_ram = bcm2835_vc_get_memory(BCM2835_VC_TAG_GET_ARM_MEMORY) / 1024 / 1024;	///< MB
	handle_rc(arm_ram);
	printf("ARM RAM : %d MB\n", (int) arm_ram);

	vc_ram = bcm2835_vc_get_memory(BCM2835_VC_TAG_GET_VC_MEMORY) / 1024 / 1024;	///< MB
	handle_rc(vc_ram);
	printf("VC RAM  : %d MB\n", (int) vc_ram);

	arm_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_ARM);
	handle_rc(arm_clock);
	printf("ARM CLOCK  : %d MHz\n", (int) arm_clock);

	vc_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_CORE);
	handle_rc(vc_clock);
	printf("VC CLOCK   : %d MHz\n", (int) vc_clock);

	uart_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_UART);
	handle_rc(uart_clock);
	printf("UART CLOCK : %d MHz\n", (int) uart_clock);

	handle_rc(bcm2835_vc_get_board_mac_address(mac_address));
	printf("MAC : %.2x%.2x:%.2x%.2x%.2x%.2x\n\n", (unsigned int) mac_address[0], (unsigned int) mac_address[1], (unsigned int) mac_address[2], (unsigned int) mac_address[3], (unsigned int) mac_address[4], (unsigned int) mac_address[5]);

#ifndef NDEBUG
	printf("Framebuffer address  : %p\n", console_get_address());
	printf("Virtual GPIO address : %p\n\n", bcm2837_gpio_virt_get_address());
#endif

	for (n = 0; n < 6; n++) {							// Loop for GPFSEL0 to GPFSEL5
		val = *((uint32_t *) BCM2835_GPIO_BASE + n) & (n < 5 ? 0x3fffffff : 0x00000fff);
		printf("GPFSEL%d ", (int) n);

		for (i = 0; i < 10; i++) {						// Loop for the 9 bit-triplets in each GPFSELn register
			uint32_t fsel_num = n * 10 + i;				// The logical FSEL number
			uint32_t bit_pos = i * 3;					// The position of the least significant bit in the triplet
			uint32_t triplet = (val >> bit_pos) & 0x7;	// Extract the bit- triplet

			if (triplet == 0) {
				int in_value;
				if (fsel_num <= 31) {
					uint32_t value = BCM2835_GPIO->GPLEV0;
					in_value = (value & (1 << fsel_num)) ? 1 : 0;
				} else {
					uint32_t value = BCM2835_GPIO->GPLEV1;
					in_value = (value & (1 << (fsel_num - 32))) ? 1 : 0;
				}
				printf("%2d:%s:%d ", (int) fsel_num, gpfsel_names[triplet], in_value);
			} else {
				printf("%2d:%s ", (int) fsel_num, gpfsel_names[triplet]);
			}

			if (n == 5 && i == 3) {						// break out of loop after FSEL53 (last triplet)
				break;
			}
		}

		(void) console_puts("\n");
	}

	(void) console_puts("\n");

	if (lcd_detect()) {
		(void) console_puts("I2C LCD 16x2 found\n");
	}

	if (oled_start(&oled_info)) {
		(void) console_puts("OLED display found\n");
		oled_puts(&oled_info, hardware_board_get_model());
	}

	irq_timer_init();
	irq_timer_set(IRQ_TIMER_3, led_handler);
	BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t) 1000000;

	if (wifi_detect()) {
		(void) console_puts("\nESP8266 information\n");
		printf(" SDK      : %s\n", system_get_sdk_version());
		printf(" Firmware : %s\n\n", wifi_get_firmware_version());
	} else {
		console_save_color();
		console_set_fg_color(CONSOLE_RED);
		(void) console_puts("\nFailed connecting to ESP8266\n");
		console_restore_color();
	}

	for(;;) {
	}

}

