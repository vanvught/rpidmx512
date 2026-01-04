/**
 * @file serial.cpp
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "serial/serial.h"
#include "hal_uart.h"
#include "hal_i2c.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

Serial::Serial()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    uart_configuration_.baud = 115200;
    uart_configuration_.bits = hal::uart::BITS_8;
    uart_configuration_.parity = hal::uart::PARITY_NONE;
    uart_configuration_.stop_bits = hal::uart::STOP_1BIT;

    spi_configuration_.speed_hz = 1000000; // 1MHz
    spi_configuration_.mode = 0;

    i2c_configuration_.address = 0x30;
    i2c_configuration_.speed_hz = HAL_I2C::FULL_SPEED;

    DEBUG_EXIT();
}

Serial::~Serial()
{
    DEBUG_ENTRY();

    s_this = nullptr;

    DEBUG_EXIT();
}

bool Serial::Init()
{
    DEBUG_ENTRY();

    if (type_ == serial::Type::kUart)
    {
        return InitUart();
    }

    if (type_ == serial::Type::kSpi)
    {
        return InitSpi();
    }

    if (type_ == serial::Type::kI2C)
    {
        return InitI2c();
    }

    return false;

    DEBUG_EXIT();
}

void Serial::Send(const uint8_t* data, uint32_t length)
{
    DEBUG_ENTRY();
    debug::Dump(const_cast<uint8_t*>(data), length);

    if (type_ == serial::Type::kUart)
    {
        SendUart(data, length);
        return;
    }

    if (type_ == serial::Type::kSpi)
    {
        SendSpi(data, length);
        return;
    }

    if (type_ == serial::Type::kI2C)
    {
        SendI2c(data, length);
        return;
    }

    DEBUG_EXIT();
}

void Serial::Print()
{
    printf("Serial [%s]\n", serial::GetType(type_));

    switch (type_)
    {
        case serial::Type::kUart:
            printf(" Baud     : %d\n", uart_configuration_.baud);
            printf(" Bits     : %d\n", uart_configuration_.bits);
            printf(" Parity   : %s\n", serial::uart::GetParity(uart_configuration_.parity));
            printf(" StopBits : %d\n", uart_configuration_.stop_bits);
            break;
        case serial::Type::kSpi:
            printf(" Speed : %d Hz\n", spi_configuration_.speed_hz);
            printf(" Mode  : %d\n", spi_configuration_.mode);
            break;
        case serial::Type::kI2C:
            printf(" Address    : %.2x\n", i2c_configuration_.address);
            printf(" Speed mode : %s\n", serial::i2c::GetSpeedMode(i2c_configuration_.speed_hz));
            break;
        default:
            break;
    }

    return;
}
