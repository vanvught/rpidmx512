/**
 * @file mcp23x17.c
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

#ifndef MCP23X17_H_
#define MCP23X17_H_

#define MCP23X17_IODIRA			0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_IODIRB			0x01	///< I/O DIRECTION (IODIRB) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_IPOLA			0x02	///< INPUT POLARITY (IPOLA) REGISTER, 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)
#define MCP23X17_IPOLB			0x03	///< INPUT POLARITY (IPOLB) REGISTER, 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)
#define MCP23X17_GPINTENA		0x04	///< INTERRUPT-ON-CHANGE CONTROL (GPINTENA) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
#define MCP23X17_GPINTENB		0x05	///< INTERRUPT-ON-CHANGE CONTROL (GPINTENB) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
#define MCP23X17_DEFVALA		0x06	///< DEFAULT COMPARE (DEFVALA) REGISTER FOR INTERRUPT-ON-CHANGE, Opposite of what is here will trigger an interrupt (default = 0)
#define MCP23X17_DEFVALB		0x07	///< DEFAULT COMPARE (DEFVALB) REGISTER FOR INTERRUPT-ON-CHANGE, Opposite of what is here will trigger an interrupt (default = 0)
#define MCP23X17_INTCONA		0x08	///< INTERRUPT CONTROL (INTCONA) REGISTER, 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)
#define MCP23X17_INTCONB		0x09	///< INTERRUPT CONTROL (INTCONB) REGISTER. 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)
#define MCP23X17_IOCON			0x0A	///< CONFIGURATION (IOCON) REGISTER
//								0x0B	///< CONFIGURATION (IOCON) REGISTER
#define MCP23X17_GPPUA			0x0C	///< PULL-UP RESISTOR CONFIGURATION (GPPUA) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23X17_GPPUB			0x0D	///< PULL-UP RESISTOR CONFIGURATION (GPPUB) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23X17_INTFA			0x0E	///< INTERRUPT FLAG (INTFA) REGISTER, READ ONLY: 1 = This Pin Triggered the Interrupt
#define MCP23X17_INTFB			0x0F	///< INTERRUPT FLAG (INTFB) REGISTER, READ ONLY: 1 = This Pin Triggered the Interrupt
#define MCP23X17_INTCAPA		0x10	///< INTERRUPT CAPTURE (INTCAPA) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
#define MCP23X17_INTCAPB		0x11	///< INTERRUPT CAPTURE (INTCAPB) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
#define MCP23X17_GPIOA			0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
#define MCP23X17_GPIOB			0x13	///< PORT (GPIOB) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
#define MCP23X17_OLATA			0x14	///< OUTPUT LATCH REGISTER (OLATA), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value
#define MCP23X17_OLATB			0x15	///< OUTPUT LATCH REGISTER (OLATB), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value

#define MCP23X17_CMD_WRITE		0x40
#define MCP23X17_CMD_READ		0x41

#define MCP23X17_IOCON_HAEN		(uint8_t)(1 << 3)

#endif /* MCP23X17_H_ */
