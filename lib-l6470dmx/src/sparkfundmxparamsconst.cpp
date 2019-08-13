/**
 * @file sparkfundmxparamsconst.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "sparkfundmxparamsconst.h"

alignas(uint32_t) const char SparkFunDmxParamsConst::FILE_NAME[] = "sparkfun.txt";
alignas(uint32_t) const char SparkFunDmxParamsConst::POSITION[] = "sparkfun_position";
#if !defined (H3)
 alignas(uint32_t) const char SparkFunDmxParamsConst::SPI_CS[] = "sparkfun_spi_cs";
#endif
alignas(uint32_t) const char SparkFunDmxParamsConst::RESET_PIN[] = "sparkfun_reset_pin";
alignas(uint32_t) const char SparkFunDmxParamsConst::BUSY_PIN[] = "sparkfun_busy_pin";
