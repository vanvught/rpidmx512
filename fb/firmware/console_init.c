#include <stdint.h>
#include "fb.h"

extern void bcm2835_uart_begin(void);
extern void bcm2835_uart_send(uint32_t c);

static const char lowercase[] = "0123456789abcdef";

void puthex(uint32_t val)
{
	for(int i = 7; i >= 0; i--)
		bcm2835_uart_send(lowercase[(val >> (i * 4)) & 0xf]);
}

void mini_uart_sendstr(const char *s) {
	char c;
	while((c = *s++)) bcm2835_uart_send(c);
}

void bcm2835_console_begin(void) {
	bcm2835_uart_begin();

	int result = fb_init();
	if (result == 0) {
		mini_uart_sendstr("Successfully set up frame buffer, fb_addr:");
		puthex((uint32_t)fb_get_framebuffer());
		mini_uart_sendstr("\r\n");
	} else {
		mini_uart_sendstr("Error setting up framebuffer: ");
		puthex(result);
		mini_uart_sendstr("\r\n");
	}
}
