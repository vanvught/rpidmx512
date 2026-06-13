/**
 * @file i2c.h
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

#ifndef I2C_H_
#define I2C_H_

#include <cstdint>

#include "timing.h"

namespace i2c {
inline constexpr uint32_t kNormalSpeed = 100000;
inline constexpr uint32_t kFullSpeed = 400000;

struct ReturnCode {
    static constexpr uint8_t kOk = 0;
    static constexpr uint8_t kNok = 1;
    static constexpr uint8_t kNack = 2;
    static constexpr uint8_t kNokLa = 3;
    static constexpr uint8_t kNokTout = 4;
};

void Begin();
void SetBaudrate(uint32_t baudrate);
void SetAddress(uint8_t address);
bool IsConnected(uint8_t address, uint32_t baudrate = kNormalSpeed);
uint8_t Read(char* buffer, uint32_t length);
uint8_t Write(const char* buffer, uint32_t length);
void WriteReg(uint8_t reg, uint8_t value);
void WriteReg(uint8_t reg, uint16_t value);
void ReadReg(uint8_t reg, uint8_t& value);
} // namespace i2c

class I2c {
   public:
    explicit I2c(uint8_t address, uint32_t baud_rate = i2c::kFullSpeed) : address_(address), baudrate_(baud_rate) {}

    uint8_t GetAddress() const { return address_; }
    uint32_t GetBaudrate() const { return baudrate_; }

    bool IsConnected() { return i2c::IsConnected(address_, baudrate_); }

    void Write(uint8_t data, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(data)};
        if (do_setup) Setup();
        i2c::Write(kBuffer, 1);
    }

    void Write(const char* data, uint32_t length) {
        Setup();
        i2c::Write(data, length);
    }

    void WriteRegister(uint8_t reg, uint8_t value, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(reg), static_cast<char>(value)};

        if (do_setup) Setup();
        i2c::Write(kBuffer, 2);
    }

    void WriteRegister(uint8_t reg, uint16_t value, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(reg), static_cast<char>(value >> 8), static_cast<char>(value & 0xFF)};

        if (do_setup) Setup();
        i2c::Write(kBuffer, 3);
    }

    uint8_t Read(bool do_setup) {
        char buf[1] = {0};

        if (do_setup) Setup();
        i2c::Read(buf, 1);

        return static_cast<uint8_t>(buf[0]);
    }

    uint8_t Read(char* buffer, uint32_t length, bool do_setup) {
        if (do_setup) Setup();
        return i2c::Read(buffer, length);
    }

    uint16_t Read16(bool do_setup) {
        char buffer[2] = {0};

        if (do_setup) Setup();
        i2c::Read(buffer, 2);

        return static_cast<uint16_t>(static_cast<uint16_t>(buffer[0]) << 8 | static_cast<uint16_t>(buffer[1]));
    }

    uint8_t ReadRegister(uint8_t reg, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(reg)};

        if (do_setup) Setup();
        i2c::Write(&kBuffer[0], 1);

        return Read(false);
    }

    uint16_t ReadRegister16(uint8_t reg, bool do_setup) {
        const char kBuf[] = {static_cast<char>(reg)};

        if (do_setup) Setup();
        i2c::Write(&kBuf[0], 1);

        return Read16(false);
    }

    uint16_t ReadRegister16DelayUs(uint8_t reg, uint32_t delay_us) {
        char buffer[2] = {0};

        buffer[0] = static_cast<char>(reg);

        Setup();
        i2c::Write(&buffer[0], 1);

        timing::DelayUs(delay_us);

        i2c::Read(buffer, 2);

        return static_cast<uint16_t>(static_cast<uint16_t>(buffer[0]) << 8 | static_cast<uint16_t>(buffer[1]));
    }

    bool AckRead() {
        char buf;
        return i2c::Read(&buf, 1) == 0;
    }

    void Setup() {
        i2c::SetAddress(address_);
        i2c::SetBaudrate(baudrate_);
    }

   private:
    uint8_t address_;
    uint32_t baudrate_;
};
#endif // I2C_H_
