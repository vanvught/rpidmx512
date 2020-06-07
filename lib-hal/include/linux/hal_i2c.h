/**
 * @file hal_i2c.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (RASPPI)

#define bcm2835_i2c_set_address bcm2835_i2c_setSlaveAddress

#else

# define FUNC_PREFIX(x) x
# include <stdint.h>
# ifdef __cplusplus
extern "C" {
# endif
  inline static void i2c_set_baudrate(__attribute__((unused)) uint32_t _q) {}
  inline static void i2c_set_address(__attribute__((unused)) uint32_t _q) {}
  inline static uint8_t i2c_write(__attribute__((unused)) const char *_p, __attribute__((unused)) uint32_t _q) { return 1;}
  inline static uint8_t i2c_read(__attribute__((unused)) const char *_p, __attribute__((unused)) uint32_t _q) { return 1;}
# ifdef __cplusplus
}
# endif

#endif

#endif /* LINUX_HAL_I2C_H_ */
