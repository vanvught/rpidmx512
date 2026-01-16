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

#if !defined(NODE_RDMNET_LLRP_ONLY)
#error
#endif

#include <cstdio>
#include <cstring>
#include <time.h>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "hal_statusled.h"
#include "hwclock.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "rdmnetdevice.h"
#include "firmwareversion.h"
#include "software_version.h"

namespace hal
{
void RebootHandler()
{
    if (!RemoteConfig::Get()->IsReboot())
    {
        Display::Get()->SetSleep(false);
        Display::Get()->Cls();
        Display::Get()->TextStatus("Rebooting ...");
    }
}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    DisplayUdf display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("RDMNet LLRP device only");

	RDMNetDevice llrp_only_device;
	llrp_only_device.Print();

    RemoteConfig remote_config(remoteconfig::Output::CONFIG, 0);

    display.SetTitle("LLRP Only - TFTP");
    display.Set(2, displayudf::Labels::kHostname);
    display.Set(3, displayudf::Labels::kIp);
    display.Set(4, displayudf::Labels::kDefaultGateway);
    display.Set(5, displayudf::Labels::kVersion);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    display.Write(6, "mDNS enabled");
    display.TextStatus("Device running", console::Colours::kConsoleGreen);

    auto t1 = time(nullptr);
    struct tm hw_clock;
    memset(&hw_clock, 0, sizeof(struct tm));
	
	hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

    for (;;)
    {
        network::Run();
        display.Run();
        hal::Run();

        time_t ltime;
        auto t2 = time(&ltime);
        if (t1 != t2)
        {
            t1 = t2;
            auto* tm = localtime(&ltime);
            struct tm tmlocal;
            memcpy(&tmlocal, tm, sizeof(struct tm));
            HwClock::Get()->Get(&hw_clock);
            display.Printf(7, "%.2d:%.2d:%.2d %.2d:%.2d:%.2d", tmlocal.tm_hour, tmlocal.tm_min, tmlocal.tm_sec, hw_clock.tm_hour, hw_clock.tm_min, hw_clock.tm_sec);
            printf("%.2d:%.2d:%.2d %.2d:%.2d:%.2d\r", tmlocal.tm_hour, tmlocal.tm_min, tmlocal.tm_sec, hw_clock.tm_hour, hw_clock.tm_min, hw_clock.tm_sec);
        }
    }
}
