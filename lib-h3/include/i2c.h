/**
 * @file i2c.h
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(CONFIG_I2C_OPTIMIZE_O2) || defined(CONFIG_I2C_OPTIMIZE_O3)
#pragma GCC push_options
#if defined(CONFIG_I2C_OPTIMIZE_O2)
#pragma GCC optimize("O2")
#else
#pragma GCC optimize("O3")
#endif
#endif

#include <cstdint>

#include "h3_i2c.h"
#include "timing.h"

namespace i2c {
inline constexpr uint32_t kNormalSpeed = h3::kI2CNormalSpeed;
inline constexpr uint32_t kFullSpeed = h3::kI2CFullSpeed;

struct ReturnCode {
    static constexpr uint8_t kOk = H3_I2C_OK;
    static constexpr uint8_t kNok = H3_I2C_NOK;
    static constexpr uint8_t kNack = H3_I2C_NACK;
    static constexpr uint8_t kNokLa = H3_I2C_NOK_LA;
    static constexpr uint8_t kNokTout = H3_I2C_NOK_TOUT;
};

inline void Begin() {
    H3I2cBegin();
}

inline void SetBaudrate(uint32_t baudrate) {
    H3I2cSetBaudrate(baudrate);
}

inline void SetAddress(uint8_t address) {
    H3I2cSetAddress(address);
}

inline uint8_t Write(const char* buffer, uint32_t length) {
    return H3I2cWrite(buffer, length);
}

inline void WriteReg(uint8_t reg, uint8_t value) {
    H3I2cWriteReg(reg, value);
}

inline void WriteReg(uint8_t reg, uint16_t value) {
    H3I2cWriteReg(reg, value);
}

inline uint8_t Read(char* buffer, uint32_t length) {
    return H3I2cRead(buffer, length);
}

inline bool IsConnected(uint8_t address, uint32_t baudrate = kNormalSpeed) {
    return H3I2cIsConnected(address, baudrate);
}

inline void ReadReg(uint8_t reg, uint8_t& value) {
    H3I2cReadReg(reg, value);
}
} // namespace i2c

class I2c {
   public:
    explicit I2c(uint8_t address, uint32_t baud_rate = i2c::kFullSpeed) : address_(address), baudrate_(baud_rate) {
		H3I2cBegin();
	}

    uint8_t GetAddress() const { return address_; }
    uint32_t GetBaudrate() const { return baudrate_; }

    bool IsConnected() { return H3I2cIsConnected(address_, baudrate_); }

    void Write(uint8_t data, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(data)};
        if (do_setup) Setup();
        H3I2cWrite(kBuffer, 1);
    }

    void Write(const char* data, uint32_t length) {
        Setup();
        H3I2cWrite(data, length);
    }

    void WriteRegister(uint8_t reg, uint8_t value, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(reg), static_cast<char>(value)};

        if (do_setup) Setup();
        H3I2cWrite(kBuffer, 2);
    }

    void WriteRegister(uint8_t reg, uint16_t value, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(reg), static_cast<char>(value >> 8), static_cast<char>(value & 0xFF)};

        if (do_setup) Setup();
        H3I2cWrite(kBuffer, 3);
    }

    uint8_t Read(bool do_setup) {
        char buf[1] = {0};

        if (do_setup) Setup();
        H3I2cRead(buf, 1);

        return static_cast<uint8_t>(buf[0]);
    }

    uint8_t Read(char* buffer, uint32_t length, bool do_setup) {
        if (do_setup) Setup();
        return H3I2cRead(buffer, length);
    }

    uint16_t Read16(bool do_setup) {
        char buffer[2] = {0};

        if (do_setup) Setup();
        H3I2cRead(buffer, 2);

        return static_cast<uint16_t>(static_cast<uint16_t>(buffer[0]) << 8 | static_cast<uint16_t>(buffer[1]));
    }

    uint8_t ReadRegister(uint8_t reg, bool do_setup) {
        const char kBuffer[] = {static_cast<char>(reg)};

        if (do_setup) Setup();
        H3I2cWrite(&kBuffer[0], 1);

        return Read(false);
    }

    uint16_t ReadRegister16(uint8_t reg, bool do_setup) {
        const char kBuf[] = {static_cast<char>(reg)};

        if (do_setup) Setup();
        H3I2cWrite(&kBuf[0], 1);

        return Read16(false);
    }

    uint16_t ReadRegister16DelayUs(uint8_t reg, uint32_t delay_us) {
        char buffer[2] = {0};

        buffer[0] = static_cast<char>(reg);

        Setup();
        H3I2cWrite(&buffer[0], 1);

        timing::DelayUs(delay_us);

        H3I2cRead(buffer, 2);

        return static_cast<uint16_t>(static_cast<uint16_t>(buffer[0]) << 8 | static_cast<uint16_t>(buffer[1]));
    }

    bool AckRead() {
        char buf;
        return H3I2cRead(&buf, 1) == 0;
    }

    void Setup() {
        H3I2cSetAddress(address_);
        H3I2cSetBaudrate(baudrate_);
    }

   private:
    uint8_t address_;
    uint32_t baudrate_;
};

#if defined(CONFIG_I2C_OPTIMIZE_O2) || defined(CONFIG_I2C_OPTIMIZE_O3)
#pragma GCC pop_options
#endif

#endif // I2C_H_
