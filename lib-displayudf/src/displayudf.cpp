/**
 * @file displayudf.cpp
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdarg>
#include <cassert>

#include "displayudf.h"
#include "firmware/debug/debug_debug.h"

DisplayUdf::DisplayUdf()
{
    assert(s_this == nullptr);
    s_this = this;

    for (uint32_t i = 0; i < static_cast<uint32_t>(displayudf::Labels::kUnknown); i++)
    {
        labels_[i] = static_cast<uint8_t>(i + 1);
    }
}

void DisplayUdf::SetTitle(const char* format, ...)
{
    va_list arp;
    va_start(arp, format);

    const auto kI = vsnprintf(title_, sizeof(title_) / sizeof(title_[0]) - 1, format, arp);

    va_end(arp);

    title_[kI] = '\0';

    DEBUG_PUTS(title_);
}

void DisplayUdf::Set(uint32_t line, displayudf::Labels label)
{
    if (!((line > 0) && (line <= displayudf::kLabelMaxRows)))
    {
        return;
    }

    for (uint32_t i = 0; i < static_cast<uint32_t>(displayudf::Labels::kUnknown); i++)
    {
        if (labels_[i] == static_cast<uint8_t>(line))
        {
            labels_[i] = labels_[static_cast<uint32_t>(label)];
            break;
        }
    }

    labels_[static_cast<uint32_t>(label)] = static_cast<uint8_t>(line);
}

void DisplayUdf::Show()
{
#if defined(NODE_ARTNET)
    ShowArtNetNode();
#elif defined(NODE_E131)
    ShowE131Bridge();
#elif defined(NODE_NODE)
    ShowNode();
#endif

    for (uint32_t i = 0; i < static_cast<uint32_t>(displayudf::Labels::kUnknown); i++)
    {
        if (labels_[i] > displayudf::kLabelMaxRows)
        {
            labels_[i] = 0xFF;
        }

        DEBUG_PRINTF("labels_[%d]=%d", i, labels_[i]);
    }

    ClearEndOfLine();
    Write(labels_[static_cast<uint32_t>(displayudf::Labels::kTitle)], title_);
    uint8_t hw_text_length;
    ClearEndOfLine();
    Write(labels_[static_cast<uint32_t>(displayudf::Labels::kBoardname)], hal::BoardName(hw_text_length));
    ClearEndOfLine();
    Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kVersion)], "Firmware V%.*s", firmwareversion::length::kSoftwareVersion,
           FirmwareVersion::Get()->GetVersion()->software_version);

#if defined(RDM_RESPONDER)
    ShowDmxStartAddress();
#endif

#if !defined(NO_EMAC)
    ShowIpAddress();
    ShowGatewayIp();
    ShowNetmask();
    ShowHostName();
#endif
}