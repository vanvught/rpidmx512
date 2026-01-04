/**
 * @file hal_i2c.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_HAL_I2C_H_
#define LINUX_HAL_I2C_H_

#include <cstdint>

#if defined(RASPPI)
#define bcm2835_I2cSetAddress bcm2835_i2c_setSlaveAddress
#define bcm2835_I2cWriteReg I2cWriteReg
#define bcm2835_I2cReadReg I2cReadReg
#define bcm2835_I2cIsConnected I2cIsConnected
bool I2cIsConnected(const uint8_t, const uint32_t);
void I2cSetBaudrate(uint32_t);
void I2cSetAddress(uint8_t);
uint8_t I2cWrite(const char*, uint32_t);
uint8_t I2cRead(char*, uint32_t);
void I2cWriteReg(const uint8_t, const uint8_t);
void I2cReadReg(const uint8_t, uint8_t&);
#if defined(LINUX_HAVE_I2C)
#define bcm2835_I2cBegin I2cBegin
#define bcm2835_I2cSetBaudrate I2cSetBaudrate
#define bcm2835_i2c_setSlaveAddress I2cSetAddress
#define bcm2835_I2cRead I2cRead
#define bcm2835_I2cWrite I2cWrite
void I2cBegin();
#endif
#else
#define FUNC_PREFIX(x) x
#if defined(LINUX_HAVE_I2C)
void I2cBegin();
void I2cSetBaudrate(uint32_t);
void I2cSetAddress(uint8_t);
uint8_t I2cWrite(const char*, uint32_t);
uint8_t I2cRead(char*, uint32_t);
bool I2cIsConnected(const uint8_t, const uint32_t);
void I2cWriteReg(const uint8_t, const uint8_t);
void I2cReadReg(const uint8_t, uint8_t&);
#else
inline static void I2cBegin() {}
inline static void I2cSetBaudrate([[maybe_unused]] uint32_t _q) {}
inline static void I2cSetAddress([[maybe_unused]] uint8_t _q) {}
inline static uint8_t I2cWrite([[maybe_unused]] const char* _p, [[maybe_unused]] uint32_t _q)
{
    return 1;
}
inline static uint8_t I2cRead([[maybe_unused]] char* _p, [[maybe_unused]] uint32_t _q)
{
    return 1;
}
bool I2cIsConnected(const uint8_t, const uint32_t);
void I2cWriteReg(const uint8_t, const uint8_t);
void I2cReadReg(const uint8_t, uint8_t&);
#endif
#endif

#endif  // LINUX_HAL_I2C_H_
