/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <stdint.h>
#include "fb.h"

extern void bcm2835_uart_begin(void);
extern void mini_uart_sendstr(const char *s);
extern void mini_uart_puthex(uint32_t val);

extern uint32_t fb_addr;

void bcm2835_console_begin(void) {
	bcm2835_uart_begin();

	int result = fb_init();
	if (result == 0) {
		mini_uart_sendstr("Successfully set up frame buffer, fb_addr:");
		mini_uart_puthex((uint32_t)fb_addr);
		mini_uart_sendstr("\r\n");
	} else {
		mini_uart_sendstr("Error setting up framebuffer: ");
		mini_uart_puthex(result);
		mini_uart_sendstr("\r\n");
	}
}
