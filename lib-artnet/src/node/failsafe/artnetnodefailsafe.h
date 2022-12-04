/**
 * @file artnetnodefailsafe.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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


#ifndef ARTNETNODEFAILSAFE_H_
#define ARTNETNODEFAILSAFE_H_

#include <cstdint>

#include "artnetnode.h"

namespace artnetnode {
namespace failsafe {
static constexpr auto BYTES_NEEDED = artnetnode::MAX_PORTS * lightset::dmx::UNIVERSE_SIZE;
}  // namespace failsafe

void failsafe_write_start();
void failsafe_write(uint32_t nPortIndex, const uint8_t *pData);
void failsafe_write_end();

void failsafe_read_start();
void failsafe_read(uint32_t nPortIndex, uint8_t *pData);
void failsafe_read_end();
}  // namespace artnetnode

#endif /* ARTNETNODEFAILSAFE_H_ */
