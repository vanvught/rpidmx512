/**
 * @file mcp23s08.h
 *
 */
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

#ifndef MCP23S08_H_
#define MCP23S08_H_

#include <stdint.h>
#include <device_info.h>

#define MCP23S08_OK						0
#define MCP23S08_ERROR					1

#define MCP23S08_DEFAULT_SLAVE_ADDRESS	0x00

#define MCP23S08_IODIR					0x00
#define MCP23S08_IPOL					0x01
#define MCP23S08_GPINTEN				0x02
#define MCP23S08_DEFVAL					0x03
#define MCP23S08_INTCON					0x04
#define MCP23S08_IOCON					0x05
#define MCP23S08_GPPU					0x06
#define MCP23S08_INTF					0x07
#define MCP23S08_INTCAP					0x08
#define MCP23S08_GPIO					0x09
#define MCP23S08_OLAT					0x0A

#define MCP23S08_CMD_WRITE				0x40
#define MCP23S08_CMD_READ				0x41

#define MCP23S08_IOCON_HAEN				(uint8_t)(1 << 3)

typedef enum {
	MCP23S08_PIN_GP0 = 0b00000001,
	MCP23S08_PIN_GP1 = 0b00000010,
	MCP23S08_PIN_GP2 = 0b00000100,
	MCP23S08_PIN_GP3 = 0b00001000,
	MCP23S08_PIN_GP4 = 0b00010000,
	MCP23S08_PIN_GP5 = 0b00100000,
	MCP23S08_PIN_GP6 = 0b01000000,
	MCP23S08_PIN_GP7 = 0b10000000
} mcp23s08Pin;

typedef enum {
	MCP23S08_FSEL_OUTP = 0b000,   ///< Output
	MCP23S08_FSEL_INPT = 0b001,   ///< Input
} mcp23s08FunctionSelect;

extern uint8_t mcp23s08_start(device_info_t *);
extern void mcp23s08_end (void);
extern uint8_t mcp23s08_reg_read(const device_info_t *, const uint8_t);
extern void mcp23s08_reg_write(const device_info_t *, const uint8_t, const uint8_t);
extern void mcp23s08_gpio_fsel(const device_info_t *, const uint8_t, const uint8_t);
extern void mcp23s08_gpio_set(const device_info_t *, const uint8_t);
extern void mcp23s08_gpio_clr(const device_info_t *, const uint8_t);

#endif /* MCP23S08_H_ */
