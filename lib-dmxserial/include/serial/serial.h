/**
 * @file serial.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SERIAL_SERIAL_H_
#define SERIAL_SERIAL_H_

#include <cassert>
#include <cstdint>
#include <cstring>

#include "common/utils/utils_enum.h"
#include "hal_spi.h"
#include "hal_i2c.h"

namespace serial
{
enum class Type : uint8_t
{
    kUart,
    kSpi,
    kI2C,
    kUndefined
};

namespace uart
{
enum class Parity : uint8_t
{
    kNone,
    kOdd,
    kEven,
    kUndefined
};

inline constexpr const char kParities[static_cast<uint32_t>(serial::uart::Parity::kUndefined)][5] = {"none", "odd", "even"};

inline const char* GetParity(Parity parity)
{
    if (parity < Parity::kUndefined)
    {
        return kParities[static_cast<uint32_t>(parity)];
    }

    return "UNDEFINED";
}

inline const char* GetParity(uint32_t parity)
{
    return GetParity(static_cast<Parity>(parity));
}

inline Parity GetParity(const char* string)
{
    assert(string != nullptr);
    uint32_t index = 0;

    for (const char(&parity)[5] : kParities)
    {
        if (strcasecmp(string, parity) == 0)
        {
            return static_cast<Parity>(index);
        }
        ++index;
    }

    return Parity::kUndefined;
}

} // namespace uart

namespace i2c
{
enum class Speed : uint8_t
{
    kNormal,
    kFast,
    kUndefined
};

inline constexpr const char kSpeed[static_cast<uint32_t>(serial::i2c::Speed::kUndefined)][9] = {"standard", "fast"};

inline const char* GetSpeedMode(serial::i2c::Speed speed)
{
    if (speed == serial::i2c::Speed::kNormal)
    {
        return kSpeed[0];
    }

    if (speed == serial::i2c::Speed::kFast)
    {
        return kSpeed[1];
    }

    return "Unknown";
}

inline const char* GetSpeedMode(uint32_t speed)
{
    if (speed == HAL_I2C::NORMAL_SPEED)
    {
        return kSpeed[0];
    }

    if (speed == HAL_I2C::FULL_SPEED)
    {
        return kSpeed[1];
    }

    return "Unknown";
}

inline serial::i2c::Speed GetSpeedMode(const char* string)
{
    assert(string != nullptr);
    uint32_t index = 0;

    for (const char(&speed)[9] : kSpeed)
    {
        if (strcasecmp(string, speed) == 0)
        {
            return static_cast<serial::i2c::Speed>(index);
        }
        ++index;
    }

    return serial::i2c::Speed::kFast;
}

} // namespace i2c

inline constexpr const char kTypes[static_cast<uint32_t>(serial::Type::kUndefined)][5] = {"uart", "spi", "i2c"};

[[nodiscard]] inline constexpr const char* GetType(Type type)
{
	    return type < Type::kUndefined ? kTypes[static_cast<uint32_t>(type)] : "UNDEFINED";
}

inline Type GetType(const char* string)
{
    assert(string != nullptr);
    uint32_t index = 0;

    for (const char(&module)[5] : kTypes)
    {
        if (strcasecmp(string, module) == 0)
        {
            return static_cast<Type>(index);
        }
        ++index;
    }

    return Type::kUndefined;
}
} // namespace serial

class Serial
{
   public:
    Serial();
    ~Serial();

    void SetType(serial::Type type = serial::Type::kUart)
    {
        if (type < serial::Type::kUndefined)
        {
            type_ = type;
        }
    }

    serial::Type GetType() { return type_; }

    // UART
    void SetUartBaud(uint32_t baud);
    uint32_t GetUartBaud() const { return uart_configuration_.baud; }

    void SetUartBits(uint32_t bits);
    uint32_t GetUartBits() const { return uart_configuration_.bits; }

    void SetUartParity(serial::uart::Parity parity = serial::uart::Parity::kNone);
    serial::uart::Parity GetUartParity() const { return common::FromValue<serial::uart::Parity>(uart_configuration_.parity); }

    void SetUartStopBits(uint32_t stop_bits);
    uint32_t GetUartStopBits() const { return uart_configuration_.stop_bits; }

    // SPI
    void SetSpiSpeedHz(uint32_t speed_hz);
    uint32_t GetSpiSpeedHz() const { return spi_configuration_.speed_hz; }

    void SetSpiMode(uint32_t mode = SPI_MODE0);
    uint8_t GetSpiMode() const { return spi_configuration_.mode; }

    // I2C
    void SetI2cAddress(uint8_t address);
    uint8_t GetI2cAddress() const { return i2c_configuration_.address; }

    void SetI2cSpeedMode(serial::i2c::Speed speed_mode);
    uint32_t GetI2cSpeed() const { return i2c_configuration_.speed_hz; }

    bool Init();

    void Print();

    // Send the data
    void Send(const uint8_t* data, uint32_t length);

    static Serial* Get() { return s_this; }

   private:
    bool InitUart();
    void SendUart(const uint8_t* data, uint32_t length);

    bool InitSpi();
    void SendSpi(const uint8_t* data, uint32_t length);

    bool InitI2c();
    void SendI2c(const uint8_t* data, uint32_t length);

   private:
    serial::Type type_{serial::Type::kUart};
    struct
    {
        uint32_t baud;
        uint8_t bits;
        uint8_t parity;
        uint8_t stop_bits;
    } uart_configuration_;
    struct
    {
        uint32_t speed_hz;
        uint8_t mode;
    } spi_configuration_;
    struct
    {
        uint32_t speed_hz;
        uint8_t address;
    } i2c_configuration_;

    static inline Serial* s_this;
};

#endif  // SERIAL_SERIAL_H_
