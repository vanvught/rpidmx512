/**
 * @file artnetdisplay.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "dmxnode.h"

namespace artnet::display
{
void Longname(const char* long_name);
void Universe(uint32_t port_index, uint32_t universe);
void MergeMode(uint32_t port_index, dmxnode::MergeMode merge_mode);
void Outputstyle(uint32_t port_index, dmxnode::OutputStyle output_style);
void Protocol(uint32_t port_index, artnet::PortProtocol port_protocol);
void RdmEnabled(uint32_t port_index, bool is_enabled);
void Failsafe(uint8_t failsafe);
} // namespace artnet::display

#endif  // ARTNETDISPLAY_H_
