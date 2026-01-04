/**
 * @file dmxserialparams.cpp
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

#include "json/dmxserialparams.h"
#include "json/dmxserialparamsconst.h"
#include "serial/serial.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"

namespace json
{
DmxSerialParams::DmxSerialParams()
{
    ConfigStore::Instance().Copy(&store_dmxserial, &ConfigurationStore::dmx_serial);
}

void DmxSerialParams::SetType(const char* val, [[maybe_unused]] uint32_t len)
{
    store_dmxserial.type = common::ToValue(serial::GetType(val));
}

void DmxSerialParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kDmxSerialKeys);
    ConfigStore::Instance().Store(&store_dmxserial, &ConfigurationStore::dmx_serial);
}

void DmxSerialParams::Set()
{
#ifndef NDEBUG
    Dump();
#endif
}

void DmxSerialParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DmxSerialParamsConst::kFileName);
    
    printf(" %s=%s [%d\n", DmxSerialParamsConst::kType.name, serial::GetType(common::FromValue<serial::Type>(store_dmxserial.type)), store_dmxserial.type);
    // UART
    printf(" %s=%d\n", DmxSerialParamsConst::kUartBaud.name, store_dmxserial.baud);
    printf(" %s=%d\n", DmxSerialParamsConst::kUartBits.name, store_dmxserial.bits);
    printf(" %s=%s [%d]\n", DmxSerialParamsConst::kUartParity.name, serial::uart::GetParity(store_dmxserial.parity), store_dmxserial.parity);
    printf(" %s=%d\n", DmxSerialParamsConst::kUartStopbits.name, store_dmxserial.stop_bits);
	// SPI   
    printf(" %s=%d\n", DmxSerialParamsConst::kSpiSpeedHz.name, store_dmxserial.spi_speed_hz);
    printf(" %s=%d\n", DmxSerialParamsConst::kSpiMode.name, store_dmxserial.spi_mode);
    // I2C
    printf(" %s=%.2x\n", DmxSerialParamsConst::kI2CAddress.name, store_dmxserial.i2c_address);
    printf(" %s=%s [%d]\n", DmxSerialParamsConst::kI2CSpeedMode.name, serial::i2c::GetSpeedMode(common::FromValue<serial::i2c::Speed>(store_dmxserial.i2c_speed_mode)), store_dmxserial.i2c_speed_mode);
}
} // namespace json