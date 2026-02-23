/**
 * @file json_config_dmxserial.cpp
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

#include <cstdint>

#include "serial/serial.h"
#include "json/json_helpers.h"
#include "json/dmxserialparams.h"
#include "json/dmxserialparamsconst.h"
#include "common/utils/utils_hex.h"
#include "dmxserial.h"

namespace json::config
{
uint32_t GetDmxSerial(char* buffer, uint32_t length)
{    
    auto& dmxserial = *DmxSerial::Get();

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) { 
	    doc[json::DmxSerialParamsConst::kType.name] = serial::GetType(dmxserial.GetType());
	    // UART
	    doc[json::DmxSerialParamsConst::kUartBaud.name] = dmxserial.GetUartBaud();
	    doc[json::DmxSerialParamsConst::kUartBits.name] = dmxserial.GetUartBits();
	 	doc[json::DmxSerialParamsConst::kUartParity.name] = serial::uart::GetParity(dmxserial.GetUartParity());
	    doc[json::DmxSerialParamsConst::kUartStopbits.name] = dmxserial.GetUartStopBits();
	 	// SPI
	 	doc[json::DmxSerialParamsConst::kSpiSpeedHz.name] = dmxserial.GetSpiSpeedHz();
	 	doc[json::DmxSerialParamsConst::kSpiMode.name] = dmxserial.GetSpiMode();
	 	// I2C   
	    char address[3];
	    doc[json::DmxSerialParamsConst::kI2CAddress.name] = common::hex::ToStringLower<2>(address, dmxserial.GetI2cAddress());
	    doc[json::DmxSerialParamsConst::kI2CSpeedMode.name] = serial::i2c::GetSpeedMode(dmxserial.GetI2cSpeed());
    });
}

void SetDmxSerial(const char* buffer, uint32_t buffer_size)
{
    ::json::DmxSerialParams dmxserial_params;
    dmxserial_params.Store(buffer, buffer_size);
    dmxserial_params.Set();
}
} // namespace json::config