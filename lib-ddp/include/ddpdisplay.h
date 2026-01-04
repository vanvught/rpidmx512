/**
 * @file ddpdisplay.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DDPDISPLAY_H_
#define DDPDISPLAY_H_

#include <cstdint>
#include <algorithm>

#include "ddp.h"
#include "dmxnode_outputtype.h"
#include "net/ip4_address.h"

#if !defined(DMXNODE_PORTS)
#error DMXNODE_PORTS is not defined
#endif

#if !defined(CONFIG_DMXNODE_PIXEL_MAX_PORTS)
#error CONFIG_DMXNODE_PIXEL_MAX_PORTS is not defined
#endif

namespace ddpdisplay
{
namespace lightset
{
static constexpr uint32_t kMaxPorts = DMXNODE_PORTS;
} // namespace lightset
namespace configuration
{
namespace pixel
{
static constexpr uint32_t kMaxPorts = CONFIG_DMXNODE_PIXEL_MAX_PORTS;
} // namespace pixel
namespace dmx
{
#if defined OUTPUT_DMX_SEND_MULTI
static constexpr uint32_t kMaxPorts = 2;
#else
static constexpr uint32_t kMaxPorts = 0;
#endif
} // namespace dmx
static constexpr uint32_t kMaxPorts = configuration::pixel::kMaxPorts + configuration::dmx::kMaxPorts;
} // namespace configuration
} // namespace ddpdisplay

static_assert(ddpdisplay::lightset::kMaxPorts == ddpdisplay::configuration::dmx::kMaxPorts + ddpdisplay::configuration::pixel::kMaxPorts * 4,
              "Configuration errror");

class DdpDisplay
{
   public:
    DdpDisplay();
    ~DdpDisplay();

    const char* GetLongName() { return "DDP Display"; }

    void Start();
    void Stop();
    void Print();

    void SetCount(uint32_t count, uint32_t channels_per_pixel, uint32_t active_ports)
    {
        count_ = count;
        strip_data_length_ = count * channels_per_pixel;
        dmxnode_output_type_data_max_length_ = (channels_per_pixel == 4 ? 512U : 510U);
        active_ports_ = std::min(active_ports, ddpdisplay::configuration::pixel::kMaxPorts);
    }

    uint32_t GetCount() const { return count_; }

    uint32_t GetChannelsPerPixel() const { return strip_data_length_ / count_; }

    void SetOutput(DmxNodeOutputType* output_type) { dmxnode_output_type_ = output_type; }

    DmxNodeOutputType* GetOutput() const { return dmxnode_output_type_; }

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    static DdpDisplay* Get() { return s_this; }

   private:
    void CalculateOffsets();
    void HandleQuery();
    void HandleData();

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    int32_t handle_{-1};
    uint8_t* receive_buffer_{nullptr};
    uint32_t from_ip_{0};
    uint32_t count_{0};
    uint32_t strip_data_length_{0};
    uint32_t dmxnode_output_type_data_max_length_{0};
    uint32_t active_ports_{0};

    DmxNodeOutputType* dmxnode_output_type_{nullptr};

    uint8_t mac_address_[net::MAC_SIZE];

    static inline uint32_t s_port_length[ddpdisplay::lightset::kMaxPorts];
    static inline uint32_t s_offset_compare[ddpdisplay::configuration::kMaxPorts];
    static inline DdpDisplay* s_this;
};

#endif // DDPDISPLAY_H_
