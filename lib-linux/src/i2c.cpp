/**
 * @file i2c.cpp
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>

#include "i2c.h"

namespace i2c {

void Begin() {
    puts("i2c::Begin");
}

void SetBaudrate(uint32_t baudrate) {
    printf("i2c::SetBaudrate=%u\n", baudrate);
}

void SetAddress(uint8_t address) {
    printf("i2c::SetAddress=0x%.2x\n", address);
}

bool IsConnected(uint8_t address, uint32_t baudrate) {
    printf("i2c::IsConnected=0x%.2x:%u\n", address, baudrate);
    return false;
}

uint8_t Read(char* buffer, uint32_t length) {
    printf("i2c::Read=%p:%u\n", reinterpret_cast<void*>(buffer), length);
    return i2c::ReturnCode::kNack;
}

uint8_t Write(const char* buffer, uint32_t length) {
    printf("i2c::Write=%p:%u\n", reinterpret_cast<const void*>(buffer), length);
    return i2c::ReturnCode::kNack;
}

void WriteReg(uint8_t reg, uint8_t value) {
    printf("i2c::WriteReg=%u:%u[8]\n", reg, value);
}

void WriteReg(uint8_t reg, uint16_t value) {
    printf("i2c::WriteReg=%u:%u[16]\n", reg, value);
}

void ReadReg(uint8_t reg, uint8_t& value) {
    printf("i2c::ReadReg=%u:%u[w]\n", reg, value);
}
} // namespace i2c