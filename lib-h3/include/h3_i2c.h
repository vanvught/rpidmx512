/**
 * @file h3_i2c.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_I2C_H_
#define H3_I2C_H_

#include <stdint.h>

typedef enum H3_I2C_BAUDRATE
{
    H3_I2C_NORMAL_SPEED = 100000,
    H3_I2C_FULL_SPEED = 400000
} h3_i2c_baudrate_t;

typedef enum H3_I2C_RC
{
    H3_I2C_OK = 0,
    H3_I2C_NOK = 1,
    H3_I2C_NACK = 2,
    H3_I2C_NOK_LA = 3,
    H3_I2C_NOK_TOUT = 4
} h3_i2c_rc_t;

void H3I2cBegin();
void H3I2cEnd();
void H3I2cSetBaudrate(uint32_t baudrate);
void H3I2cSetAddress(uint8_t address);
uint8_t H3I2cWrite(const char*, uint32_t);
uint8_t H3I2cRead(char*, uint32_t);
bool H3I2cIsConnected(uint8_t, uint32_t baudrate = H3_I2C_NORMAL_SPEED);
void H3I2cWriteReg(const uint8_t, const uint8_t);
void H3I2cReadReg(const uint8_t, uint8_t&);

#endif  // H3_I2C_H_
