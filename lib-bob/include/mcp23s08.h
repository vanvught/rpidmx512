/**
 * @file mcp23s08.h
 *
 */
/* Copyright (C) 2014-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MCP23S08_H_
#define MCP23S08_H_

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

#define MCP23S08_DEFAULT_SLAVE_ADDRESS	0x00

#define MCP23S08_SPI_SPEED_MAX_HZ		10000000	///< 10 MHz
#define MCP23S08_SPI_SPEED_DEFAULT_HZ	 2000000	///< 2 MHz

typedef enum {
	MCP23S08_PIN_GP0 = (1 << 0),	///< 0b00000001, Bidirectional I/O pin.
	MCP23S08_PIN_GP1 = (1 << 1),	///< 0b00000010, Bidirectional I/O pin.
	MCP23S08_PIN_GP2 = (1 << 2),	///< 0b00000100, Bidirectional I/O pin.
	MCP23S08_PIN_GP3 = (1 << 3),	///< 0b00001000, Bidirectional I/O pin.
	MCP23S08_PIN_GP4 = (1 << 4),	///< 0b00010000, Bidirectional I/O pin.
	MCP23S08_PIN_GP5 = (1 << 5),	///< 0b00100000, Bidirectional I/O pin.
	MCP23S08_PIN_GP6 = (1 << 6),	///< 0b01000000, Bidirectional I/O pin.
	MCP23S08_PIN_GP7 = (1 << 7)		///< 0b10000000, Bidirectional I/O pin.
} mcp23s08Pin;

typedef enum {
	MCP23S08_FSEL_OUTP = 0,			///< Output
	MCP23S08_FSEL_INPT = 1,			///< Input
} mcp23s08FunctionSelect;

#ifdef __cplusplus
extern "C" {
#endif

extern bool mcp23s08_start(device_info_t *);
extern uint8_t mcp23s08_reg_read(const device_info_t *, uint8_t);
extern void mcp23s08_reg_write(const device_info_t *, uint8_t, uint8_t);
extern void mcp23s08_gpio_fsel(const device_info_t *, uint8_t, uint8_t);
extern void mcp23s08_gpio_set(const device_info_t *, uint8_t);
extern void mcp23s08_gpio_clr(const device_info_t *, uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* MCP23S08_H_ */
