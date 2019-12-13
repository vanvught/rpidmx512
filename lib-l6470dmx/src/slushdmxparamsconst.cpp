#if !defined(ORANGE_PI)
/**
 * @file slushdmxparamsconst.cpp
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

#include "slushdmxparamsconst.h"

alignas(uint32_t) const char SlushDmxParamsConst::FILE_NAME[] = "slush.txt";

alignas(uint32_t) const char SlushDmxParamsConst::USE_SPI[] = "use_spi_busy";

alignas(uint32_t) const char SlushDmxParamsConst::DMX_START_ADDRESS_PORT_A[] = "dmx_start_address_port_a";
alignas(uint32_t) const char SlushDmxParamsConst::DMX_FOOTPRINT_PORT_A[] = "dmx_footprint_port_a";
alignas(uint32_t) const char SlushDmxParamsConst::DMX_SLOT_INFO_PORT_A[] = "dmx_slot_info_port_a";

alignas(uint32_t) const char SlushDmxParamsConst::DMX_START_ADDRESS_PORT_B[] = "dmx_start_address_port_b";
alignas(uint32_t) const char SlushDmxParamsConst::DMX_FOOTPRINT_PORT_B[] = "dmx_footprint_port_b";
alignas(uint32_t) const char SlushDmxParamsConst::DMX_SLOT_INFO_PORT_B[] = "dmx_slot_info_port_b";

#endif /* #if !defined(ORANGE_PI) */
