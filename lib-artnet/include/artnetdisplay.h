/**
 * @file artnetdisplay.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETDISPLAY_H_
#define ARTNETDISPLAY_H_

#include <cstdint>

#include "artnet.h"
#include "lightset.h"

namespace artnet {
void display_longname(const char *pLongName);
void display_universe_switch(uint32_t nPortIndex, uint8_t nAddress);
void display_net_switch(uint8_t nAddress);
void display_subnet_switch(uint8_t nAddress);
void display_merge_mode(uint32_t nPortIndex, lightset::MergeMode mergeMode);
void display_outputstyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle);
void display_port_protocol(uint32_t nPortIndex, artnet::PortProtocol portProtocol);
void display_rdm_enabled(uint32_t nPortIndex, bool isEnabled);
void display_failsafe(uint8_t nFailsafe);
}  // namespace artnet
#endif /* ARTNETDISPLAY_H_ */
