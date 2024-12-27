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

#if defined (RASPPI)
# define bcm2835_i2c_set_address 		bcm2835_i2c_setSlaveAddress
# define bcm2835_i2c_write_register		i2c_write_register
# define bcm2835_i2c_read_register		i2c_read_register
# define bcm2835_i2c_is_connected		i2c_is_connected
  bool i2c_is_connected(const uint8_t, const uint32_t);
  void i2c_set_baudrate(uint32_t);
  void i2c_set_address(uint8_t);
  uint8_t i2c_write(const char *, uint32_t);
  uint8_t i2c_read(char *, uint32_t);
  void i2c_write_register(const uint8_t, const uint8_t);
  void i2c_read_register(const uint8_t, uint8_t&);
# if defined(LINUX_HAVE_I2C)
#  define bcm2835_i2c_begin 			i2c_begin
#  define bcm2835_i2c_set_baudrate		i2c_set_baudrate
#  define bcm2835_i2c_setSlaveAddress	i2c_set_address
#  define bcm2835_i2c_read				i2c_read
#  define bcm2835_i2c_write				i2c_write
   void i2c_begin();
# endif
#else
# define FUNC_PREFIX(x) x
# if defined(LINUX_HAVE_I2C)
   void i2c_begin();
   void i2c_set_baudrate(uint32_t);
   void i2c_set_address(uint8_t);
   uint8_t i2c_write(const char *, uint32_t);
   uint8_t i2c_read(char *, uint32_t);
   bool i2c_is_connected(const uint8_t, const uint32_t);
   void i2c_write_register(const uint8_t, const uint8_t);
   void i2c_read_register(const uint8_t, uint8_t&);
# else
   inline static void i2c_begin() {}
   inline static void i2c_set_baudrate([[maybe_unused]] uint32_t _q) {}
   inline static void i2c_set_address([[maybe_unused]] uint8_t _q) {}
   inline static uint8_t i2c_write([[maybe_unused]] const char *_p, [[maybe_unused]] uint32_t _q) { return 1;}
   inline static uint8_t i2c_read([[maybe_unused]] char *_p, [[maybe_unused]] uint32_t _q) { return 1;}
   bool i2c_is_connected(const uint8_t, const uint32_t);
   void i2c_write_register(const uint8_t, const uint8_t);
   void i2c_read_register(const uint8_t, uint8_t&);
# endif
#endif

#endif /* LINUX_HAL_I2C_H_ */
