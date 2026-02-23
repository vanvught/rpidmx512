/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <cassert>

#include "dmxmonitor.h"
#include "dmxmonitorstore.h"
#include "dmxnode.h"

DmxMonitor::DmxMonitor()
{
    assert(s_this == nullptr);
    s_this = this;

    for (uint32_t port_index = 0; port_index < dmxmonitor::output::text::kMaxPorts; port_index++)
    {
        memset(&data_[port_index], 0, sizeof(struct Data));
    }

    for (uint32_t i = 0; i < sizeof(started_); i++)
    {
        started_[i] = false;
    }
}

bool DmxMonitor::SetDmxStartAddress(uint16_t start_address)
{
    if ((start_address == 0) ||  (start_address > (512 - max_channels_)))
    {
		dmx_start_address_ = dmxnode::kStartAddressDefault;
        return false;
    }

    dmxmonitor_store::SaveDmxStartAddress(start_address);

    dmx_start_address_ = start_address;
    return true;
}

void DmxMonitor::DisplayDateTime(uint32_t port_index, const char* string)
{
    assert(port_index < dmxmonitor::output::text::kMaxPorts);

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    auto* tm = localtime(&tv.tv_sec);

    printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6d %s:%c\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec,
           static_cast<int>(tv.tv_usec), string, port_index + 'A');
}

void DmxMonitor::Start(uint32_t port_index)
{
    assert(port_index < dmxmonitor::output::text::kMaxPorts);

    if (started_[port_index])
    {
        return;
    }

    started_[port_index] = true;
    DisplayDateTime(port_index, "Start");
}

void DmxMonitor::Stop(uint32_t port_index)
{
    assert(port_index < dmxmonitor::output::text::kMaxPorts);

    if (!started_[port_index])
    {
        return;
    }

    started_[port_index] = false;
    DisplayDateTime(port_index, "Stop");
}

template <bool doUpdate> void DmxMonitor::SetData(uint32_t port_index, const uint8_t* data, uint32_t length)
{
    assert(port_index < dmxmonitor::output::text::kMaxPorts);

    if constexpr (doUpdate)
    {
        Update(port_index, data, length);
    }
    else
    {
        memcpy(data_[port_index].data, data, length);
        data_[port_index].length = length;
    }
}

void DmxMonitor::Update(uint32_t port_index, const uint8_t* data, uint32_t length)
{
    assert(port_index < dmxmonitor::output::text::kMaxPorts);

    struct timeval tv;
    uint32_t i, j;

    gettimeofday(&tv, nullptr);
    auto* tm = localtime(&tv.tv_sec);

    printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6d DMX:%c %d:%d:%d ", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec,
           static_cast<int>(tv.tv_usec), port_index + 'A', static_cast<int>(length), static_cast<int>(max_channels_), static_cast<int>(dmx_start_address_));

    for (i = static_cast<uint32_t>(dmx_start_address_ - 1), j = 0; (i < length) && (j < max_channels_); i++, j++)
    {
        switch (format_)
        {
            case dmxmonitor::Format::kPct:
                printf("%3d ", ((data[i] * 100)) / 255);
                break;
            case dmxmonitor::Format::kDec:
                printf("%3d ", data[i]);
                break;
            default:
                printf("%.2x ", data[i]);
                break;
        }
    }

    for (; j < max_channels_; j++)
    {
        printf("-- ");
    }

    puts("");
}

// Explicit template instantiations
template void DmxMonitor::SetData<true>(uint32_t, const uint8_t*, uint32_t);
template void DmxMonitor::SetData<false>(uint32_t, const uint8_t*, uint32_t);
