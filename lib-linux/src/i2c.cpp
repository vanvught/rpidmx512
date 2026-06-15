/**
 * @file i2c.cpp
 * @brief Linux
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

#include <cstdint>
#include <cstdio>
#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/i2c-dev.h>
#endif

#include "i2c.h"

namespace i2c {
#if defined(__linux__)
static constexpr char kDevice[] = "/dev/i2c-1";
int file = -1;
#endif

void Begin() {
#if defined(__linux__)
    if (file != -1) {
        close(file);
        file = -1;
    }

    file = open(kDevice, O_RDWR | O_CLOEXEC);
    if (file < 0) {
        perror(kDevice);
    }
#else
    puts("i2c::Begin");
#endif
}

void SetBaudrate([[maybe_unused]] uint32_t baudrate) {
#if defined(__linux__)
#else
    printf("i2c::SetBaudrate(%u)\n", baudrate);
#endif
}

void SetAddress(uint8_t address) {
#if defined(__linux__)
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        close(file);
        file = -1;
        perror("SetAddress");
    }
#else
    printf("i2c::SetAddress(0x%.2x)\n", address);
#endif
}

bool IsConnected(uint8_t address, uint32_t baudrate) {
#if defined(__linux__)
    if (file < 0) return false;
    SetAddress(address);
    if (file < 0) return false;
    SetBaudrate(baudrate);

    uint8_t result;
    char buffer;

    if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F)) {
        result = Read(&buffer, 1);
    } else {
        // This is known to corrupt the Atmel AT24RF08 EEPROM
        result = Write(nullptr, 0);
    }

    return (result == 0) ? true : false;
#else
    printf("i2c::IsConnected(address=0x%.2x,baudrate=%u)\n", address, baudrate);
    return false;
#endif
}

uint8_t Read(char* buffer, uint32_t length) {
#if defined(__linux__)
    if (file < 0 || buffer == nullptr || length == 0) {
        return i2c::ReturnCode::kNok;
    }

    const auto nbyte = static_cast<ssize_t>(length);

    if (read(file, buffer, static_cast<size_t>(nbyte)) != nbyte) {
        return i2c::ReturnCode::kNok;
    }

    return i2c::ReturnCode::kOk;
#else
    printf("i2c::Read(%p,%u)\n", reinterpret_cast<void*>(buffer), length);
    return i2c::ReturnCode::kNack;
#endif
}

uint8_t Write(const char* buffer, uint32_t length) {
#if defined(__linux__)
    if (file < 0) {
        return i2c::ReturnCode::kNok;
    }

    ssize_t nbyte = length;

    if (write(file, buffer, nbyte) != nbyte) {
        return i2c::ReturnCode::kNok;
    }

    return i2c::ReturnCode::kOk;
#else
    printf("i2c::Write(%p,%u)\n", reinterpret_cast<const void*>(buffer), length);
    return i2c::ReturnCode::kNack;
#endif
}

void WriteReg(uint8_t reg, uint8_t value) {
#if defined(__linux__)
    const char kBuffer[2] = {static_cast<char>(reg), static_cast<char>(value)};
    Write(kBuffer, 2);
#else
    printf("i2c::WriteReg=(%u,%u[8])\n", reg, value);
#endif
}

void WriteReg(uint8_t reg, uint16_t value) {
#if defined(__linux__)
    const char kBuffer[] = {static_cast<char>(reg), static_cast<char>(value >> 8), static_cast<char>(value & 0xFF)};

    Write(kBuffer, 3);
#else
    printf("i2c::WriteReg(%u,%u[16])\n", reg, value);
#endif
}

void ReadReg(uint8_t reg, uint8_t& value) {
#if defined(__linux__)
    char buffer[1];

    buffer[0] = static_cast<char>(reg);

    Write(buffer, 1);
    Read(buffer, 1);

    value = buffer[0];
#else
    printf("i2c::ReadReg(%u,%u[w])\n", reg, value);
#endif
}
} // namespace i2c