/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "dmxmonitor.h"
#include "h3/console_fb.h"

static constexpr uint32_t kTopRow = 2;
static constexpr uint32_t kHexColumns = 32;
static constexpr uint32_t kHexRows = 16;
static constexpr uint32_t kDecColumns = 24;
static constexpr uint32_t kDecRows = 22;

static constexpr uint16_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint16_t>(((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | (((b) & 0xFF))));
}

DmxMonitor::DmxMonitor()
{
    assert(s_this == nullptr);
    s_this = this;

    memset(data_, 0, sizeof(data_) / sizeof(data_[0]));
}

bool DmxMonitor::SetDmxStartAddress(uint16_t dmx_start_address)
{
    if (dmx_start_address != dmxnode::kStartAddressDefault)
    {
        return false;
    }

    dmx_start_address_ = dmx_start_address;
    return true;
}

void DmxMonitor::Start([[maybe_unused]] uint32_t port_index)
{
    if (started_)
    {
        return;
    }

    started_ = true;

    auto row = kTopRow;

    console::ClearLine(kTopRow);

    switch (format_)
    {
        case dmxmonitor::Format::kPct:
            console::PutChar('%');
            break;
        case dmxmonitor::Format::kDec:
            console::PutChar('D');
            break;
        default:
            console::PutChar('H');
            break;
    }

    if (format_ != dmxmonitor::Format::kDec)
    {
        console::Puts("   01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32");

        for (uint32_t i = 1; i < (kHexRows * kHexColumns); i = i + kHexColumns)
        {
            console::SetCursor(0, ++row);
            printf("%3d", i);
        }
    }
    else
    {
        console::Puts("     1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24");

        for (uint32_t i = 1; i < (kDecRows * kDecColumns); i = i + kDecColumns)
        {
            console::SetCursor(0, ++row);
            printf("%3d", i);
        }
    }

    Update();
}

void DmxMonitor::Stop([[maybe_unused]] uint32_t port_index)
{
    if (!started_)
    {
        return;
    }

    started_ = false;

    if (format_ != dmxmonitor::Format::kDec)
    {
        for (uint32_t i = (kTopRow + 1); i < (kTopRow + kHexRows + 1); i++)
        {
            console::SetCursor(4, i);
            console::Puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
        }
    }
    else
    {
        uint32_t i;
        for (i = (kTopRow + 1); i < (kTopRow + kDecRows); i++)
        {
            console::SetCursor(4, i);
            console::Puts("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        }
        console::SetCursor(4, i);
        console::Puts("--- --- --- --- --- --- --- ---");
    }
}

void DmxMonitor::Cls()
{
    uint32_t i;

    for (i = kTopRow; i < (kTopRow + kHexRows + 2); i++)
    {
        console::ClearLine(i);
    }

    if (format_ == dmxmonitor::Format::kDec)
    {
        for (; i < (kTopRow + kDecRows + 2); i++)
        {
            console::ClearLine(i);
        }
    }
}

template <bool doUpdate> void DmxMonitor::SetData([[maybe_unused]] uint32_t port_index, const uint8_t* data, uint32_t length)
{
    slots_ = static_cast<uint16_t>(length);

    memcpy(data_, data, length);

    if constexpr (doUpdate)
    {
        Update();
    }
}

void DmxMonitor::Update()
{
    auto row = kTopRow;
    uint32_t i, j;
    auto* p = data_;
    uint16_t slot = 0;

    if (format_ != dmxmonitor::Format::kDec)
    {
        for (i = 0; (i < kHexRows) && (slot < slots_); i++)
        {
            console::SetCursor(4, ++row);

            for (j = 0; (j < kHexColumns) && (slot < slots_); j++)
            {
                const auto kD = *p++;

                if (kD == 0)
                {
                    console::Puts(" 0");
                }
                else
                {
                    if (format_ == dmxmonitor::Format::kHex)
                    {
                        console::PuthexFgBg(kD, (kD > 92 ? console::Colours::kConsoleBlack : console::Colours::kConsoleWhite),
                                            static_cast<console::Colours>(RGB(kD, kD, kD)));
                    }
                    else
                    {
                        console::PutpctFgBg(static_cast<uint8_t>((kD * 100) / 255),
                                            (kD > 92 ? console::Colours::kConsoleBlack : console::Colours::kConsoleWhite),
                                            static_cast<console::Colours>(RGB(kD, kD, kD)));
                    }
                }
                console::PutChar(' ');
                slot++;
            }

            for (; j < kHexColumns; j++)
            {
                console::Puts("   ");
            }
        }

        for (; i < kHexRows; i++)
        {
            console::SetCursor(4, ++row);
            console::Puts("                                                                                               ");
        }
    }
    else
    {
        for (i = 0; (i < kDecRows) && (slot < slots_); i++)
        {
            console::SetCursor(4, ++row);

            for (j = 0; (j < kDecColumns) && (slot < slots_); j++)
            {
                const uint8_t kD = *p++;

                if (kD == 0)
                {
                    console::Puts("  0");
                }
                else
                {
                    console::Put3decFgBg(kD, (kD > 92 ? console::Colours::kConsoleBlack : console::Colours::kConsoleWhite),
                                         static_cast<console::Colours>(RGB(kD, kD, kD)));
                }
                console::PutChar(' ');
                slot++;
            }

            for (; j < kDecColumns; j++)
            {
                console::Puts("    ");
            }
        }

        for (; i < kDecRows; i++)
        {
            console::SetCursor(4, ++row);
            console::Puts("                                                                                               ");
        }
    }
}

// Explicit template instantiations
template void DmxMonitor::SetData<true>(const uint32_t, const uint8_t*, uint32_t);
template void DmxMonitor::SetData<false>(const uint32_t, const uint8_t*, uint32_t);
