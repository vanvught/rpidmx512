/**
 * @file dmxserialparamsconst.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXSERIALPARAMSCONST_H_
#define DMXSERIALPARAMSCONST_H_

struct DmxSerialParamsConst {
	static inline const char FILE_NAME[] = "serial.txt";
	static inline const char TYPE[] = "type";
	static inline const char UART_BAUD[] = "uart_baud";
	static inline const char UART_BITS[] = "uart_bits";
	static inline const char UART_PARITY[] = "uart_parity";
	static inline const char UART_STOPBITS[] = "uart_stopbits";
	static inline const char SPI_SPEED_HZ[] = "spi_speed_hz";
	static inline const char SPI_MODE[] = "spi_mode";
	static inline const char I2C_ADDRESS[] = "i2c_address";
	static inline const char I2C_SPEED_MODE[] = "i2c_speed_mode";
};

#endif /* DMXSERIALPARAMSCONST_H_ */
