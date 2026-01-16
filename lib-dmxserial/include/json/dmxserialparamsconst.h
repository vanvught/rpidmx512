/**
 * @file dmxserialparamsconst.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_DMXSERIALPARAMSCONST_H_
#define JSON_DMXSERIALPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct DmxSerialParamsConst
{
	static constexpr char kFileName[] = "dmxserial.json";
   
	static constexpr json::SimpleKey kType {
	    "type",
	    4,
	    Fnv1a32("type", 4)
	};

	// UART
	static constexpr json::SimpleKey kUartBaud {
	    "uart_baud",
	    9,
	    Fnv1a32("uart_baud", 9)
	};		  
		  
	static constexpr json::SimpleKey kUartBits {
	    "uart_bits",
	    9,
	    Fnv1a32("uart_bits", 9)
	};	
	
	static constexpr json::SimpleKey kUartParity {
	    "uart_parity",
	    11,
	    Fnv1a32("uart_parity",11)
	};	

	static constexpr json::SimpleKey kUartStopbits {
	    "uart_stopbits",
	    13,
	    Fnv1a32("uart_stopbits",13)
	};

	// SPI
	static constexpr json::SimpleKey kSpiSpeedHz {
	    "spi_speed_hz",
	    12,
	    Fnv1a32("spi_speed_hz",12)
	};	

	static constexpr json::SimpleKey kSpiMode {
	    "spi_mode",
	    8,
	    Fnv1a32("spi_mode",8)
	};
	
	// I2C
	static constexpr json::SimpleKey kI2CAddress {
	    "i2c_address",
	    11,
	    Fnv1a32("i2c_address",11)
	};	

	static constexpr json::SimpleKey kI2CSpeedMode {
	    "i2c_speed_mode",
	    14,
	    Fnv1a32("i2c_speed_mode",14)
	};	   
};
} // namespace json

#endif  // JSON_DMXSERIALPARAMSCONST_H_
