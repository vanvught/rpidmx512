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

#if defined(DEBUG_DISPLAYUDF)
#undef NDEBUG
#endif

#include <cstdint>

#include "displayudf.h"
#include "artnet.h"
#include "dmxnode.h"

namespace artnet::display
{
void Longname([[maybe_unused]] const char* long_name) {}

void Universe([[maybe_unused]] uint32_t port_index, [[maybe_unused]] uint32_t universe)
{
    DisplayUdf::Get()->ShowUniverseArtNetNode();
}

void MergeMode([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmxnode::MergeMode merge_mode)
{
    DisplayUdf::Get()->ShowUniverseArtNetNode();
}

void Outputstyle([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmxnode::OutputStyle output_style)
{
    DisplayUdf::Get()->ShowUniverseArtNetNode();
}

void Protocol([[maybe_unused]] uint32_t port_index, [[maybe_unused]] artnet::PortProtocol port_protocol)
{
    DisplayUdf::Get()->ShowUniverseArtNetNode();
}

void RdmEnabled([[maybe_unused]] uint32_t port_index, [[maybe_unused]] bool is_enabled)
{
    DisplayUdf::Get()->ShowUniverseArtNetNode();
}

void Failsafe([[maybe_unused]] uint8_t failsafe)
{
    // TODO ShowFailSafe
}
} // namespace artnet::display
