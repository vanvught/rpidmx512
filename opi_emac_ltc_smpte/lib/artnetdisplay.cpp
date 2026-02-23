/**
 * @file artnetdisplay.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnet.h"
#include "dmxnode.h"
 #include "firmware/debug/debug_debug.h"

namespace artnet::display
{
void Longname([[maybe_unused]] const char* long_name)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void Universe([[maybe_unused]] uint32_t port_index, [[maybe_unused]] uint32_t universe)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void MergeMode([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmxnode::MergeMode merge_mode)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void Outputstyle([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmxnode::OutputStyle output_style)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void Protocol([[maybe_unused]] uint32_t port_index, [[maybe_unused]] artnet::PortProtocol port_protocol)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void RdmEnabled([[maybe_unused]] uint32_t port_index, [[maybe_unused]] bool is_enabled)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void Failsafe([[maybe_unused]] uint8_t failsafe)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}
} // namespace artnet::display
