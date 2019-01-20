/**
 * @file mcp23017.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MCP23017_H_
#define MCP23017_H_

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

#define MCP23017_DEFAULT_SLAVE_ADDRESS 0x20

typedef enum {
	MCP23017_PIN_GPA0 = (1 << 0),
	MCP23017_PIN_GPA1 = (1 << 1),
	MCP23017_PIN_GPA2 = (1 << 2),
	MCP23017_PIN_GPA3 = (1 << 3),
	MCP23017_PIN_GPA4 = (1 << 4),
	MCP23017_PIN_GPA5 = (1 << 5),
	MCP23017_PIN_GPA6 = (1 << 6),
	MCP23017_PIN_GPA7 = (1 << 7),

	MCP23017_PIN_GPB0 = (1 << 8),
	MCP23017_PIN_GPB1 = (1 << 9),
	MCP23017_PIN_GPB2 = (1 << 10),
	MCP23017_PIN_GPB3 = (1 << 11),
	MCP23017_PIN_GPB4 = (1 << 12),
	MCP23017_PIN_GPB5 = (1 << 13),
	MCP23017_PIN_GPB6 = (1 << 14),
	MCP23017_PIN_GPB7 = (1 << 15)
} mcp23s17Pin;

typedef enum {
	MCP23017_FSEL_OUTP,
	MCP23017_FSEL_INPT
} mcp23017FunctionSelect;

typedef enum {
	MCP23017_GPIO_PUD_OFF,
	MCP23017_GPIO_PUD_UP
} mcp23017PUDControl;

#ifdef __cplusplus
extern "C" {
#endif

extern bool mcp23017_start(device_info_t *);

extern uint16_t mcp23017_reg_read(const device_info_t *, uint8_t);
extern void mcp23017_reg_write(const device_info_t *, uint8_t, uint16_t);

extern void mcp23017_gpio_fsel(const device_info_t *, mcp23s17Pin, mcp23017FunctionSelect);
extern void mcp23017_gpio_set(const device_info_t *, mcp23s17Pin);
extern void mcp23017_gpio_clr(const device_info_t *, mcp23s17Pin);

extern uint8_t mcp23017_gpio_lev(const device_info_t *, mcp23s17Pin);
extern void mcp23017_gpio_set_pud(const device_info_t *, mcp23s17Pin, mcp23017PUDControl);

#ifdef __cplusplus
}
#endif

#endif /* MCP23017_H_ */
