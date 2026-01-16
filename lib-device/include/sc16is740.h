/**
 * @file sc16is740.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SC16IS740_H_
#define SC16IS740_H_

#include <cstdint>

#include "hal_i2c.h"
#include "hal_millis.h"

#include "sc16is7x0.h"

namespace sc16is740
{
static constexpr uint8_t kI2CAddress = 0x4D;
static constexpr uint32_t kCristalHz = 14745600UL;
} // namespace sc16is740

class SC16IS740 : HAL_I2C
{
   public:
    enum class SerialParity
    {
        kNone,
        kOdd,
        kEven,
        kForceD0,
        kForceD1
    };

    enum class TriggerLevel
    {
        kLevelTx,
        kLevelRx
    };

    explicit SC16IS740(uint8_t address = sc16is740::kI2CAddress, uint32_t on_board_crystal_hz = sc16is740::kCristalHz);
    ~SC16IS740() = default;

    void SetOnBoardCrystal(uint32_t on_board_crystal_hz) { on_board_crystal_hz_ = on_board_crystal_hz; }

    uint32_t GetOnBoardCrystal() const { return on_board_crystal_hz_; }

    void SetFormat(uint32_t bits, SerialParity parity, uint32_t stop_bits);
    void SetBaud(uint32_t baud);

    bool IsInterrupt()
    {
        const uint32_t kRegisterIIR = ReadRegister(SC16IS7X0_IIR);

        return ((kRegisterIIR & 0x1) != 0x1);
    }

    // Read

    int GetChar()
    {
        if (!is_connected_)
        {
            return -1;
        }

        if (!IsReadable())
        {
            return -1;
        }

        return ReadRegister(SC16IS7X0_RHR);
    }

    int GetChar(uint32_t time_out)
    {
        if (!is_connected_)
        {
            return -1;
        }

        if (!IsReadable(time_out))
        {
            return -1;
        }

        return ReadRegister(SC16IS7X0_RHR);
    }

    // Write
    int PutChar(int value)
    {
        if (!is_connected_)
        {
            return -1;
        }

        while (!IsWritable())
        {
        }

        WriteRegister(SC16IS7X0_THR, static_cast<uint8_t>(value));

        return value;
    }

    // Multiple read/write

    void WriteBytes(const uint8_t* bytes, uint32_t size);
    void ReadBytes(uint8_t* bytes, uint32_t& size, uint32_t time_out);
    void FlushRead(uint32_t time_out);

   private:
    bool IsWritable() { return (ReadRegister(SC16IS7X0_TXLVL) != 0); }

    bool IsReadable() { return (ReadRegister(SC16IS7X0_RXLVL) != 0); }

    bool IsReadable(uint32_t time_out)
    {
        const auto kMillis = hal::Millis();
        do
        {
            if (IsReadable())
            {
                return true;
            }
        } while ((hal::Millis() - time_out) < kMillis);

        return false;
    }

   private:
    uint32_t on_board_crystal_hz_;
    bool is_connected_{false};
};

#endif  // SC16IS740_H_
