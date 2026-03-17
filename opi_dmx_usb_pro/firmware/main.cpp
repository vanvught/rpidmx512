/**
 * @file main.cpp
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

#include <cstdio>
#include <cstdint>

#include "hal.h"
#include "h3/hal_watchdog.h"
#include "hal_boardinfo.h"
#include "rdmdevice.h"
#include "widget.h"
#include "widgetparams.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "software_version.h"

#ifndef ALIGNED
#define ALIGNED __attribute__((aligned(4)))
#endif

static constexpr char kWidgetModeNames[4][12] ALIGNED = {"DMX_RDM", "DMX", "RDM", "RDM_SNIFFER"};
static constexpr struct rdm::device::InfoData kDeviceLabel ALIGNED = {const_cast<char*>("Orange Pi Zero DMX USB Pro"), 26};

namespace hal
{
void RebootHandler() {}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    ConfigStore config_store;
    FlashCodeInstall spiflash_install;

    Widget widget;
    widget.SetPortDirection(0, dmx::PortDirection::kInput, false);

    WidgetParams widget_params;
    widget_params.Load();
    widget_params.Set();

	auto& rdm_device = rdm::device::Device::Instance();
	rdm_device.SetLabel(&kDeviceLabel);

    const auto* device_uid = rdm::device::Base::Instance().GetUID();
    struct rdm::device::InfoData rdm_device_label;
    rdm_device.GetLabel(&rdm_device_label);
    const auto kWidgetMode = widget_params.GetMode();

    uint8_t hw_text_length;
    printf("[V%s] %s Compiled on %s at %s\n", kSoftwareVersion, hal::BoardName(hw_text_length), __DATE__, __TIME__);
    printf("RDM Controller with USB [Compatible with Enttec USB Pro protocol], Widget mode : %d (%s)\n", kWidgetMode,
           kWidgetModeNames[static_cast<uint32_t>(kWidgetMode)]);
    printf("Device UUID : %.2x%.2x:%.2x%.2x%.2x%.2x, ", device_uid[0], device_uid[1], device_uid[2], device_uid[3], device_uid[4],
           device_uid[5]);
    printf("Label : %.*s\n", static_cast<int>(rdm_device_label.length), reinterpret_cast<const char*>(rdm_device_label.data));

    hal::WatchdogInit();

    if (kWidgetMode == widget::Mode::kRdmSniffer)
    {
        widget.SetPortDirection(0, dmx::PortDirection::kInput, true);
        widget.SnifferFillTransmitBuffer(); // Prevent missing first frame
    }

    for (;;)
    {
        hal::WatchdogFeed();
        widget.Run();
        hal::Run();
    }
}
