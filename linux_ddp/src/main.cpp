/**
 * @file main.cpp
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

#include <cstdint>
#include <signal.h>

#include "hal.h"
#include "network.h"
#include "display.h"
#include "ddpdisplay.h"
#include "dmxmonitor.h"
#include "json/dmxmonitorparams.h"
#include "rdmnetdevice.h"
#include "rdmdevice.h"
#include "remoteconfig.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"

static bool keep_running = true;

void IntHandler(int)
{
    keep_running = false;
}

int main(int argc, char** argv) // NOLINT
{
    struct sigaction act;
    act.sa_handler = IntHandler;
    sigaction(SIGINT, &act, nullptr);
    hal::Init();
    Display display;
    ConfigStore config_store;
    Network nw(argc, argv);
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

    hal::print();
    fw.Print();
    nw.Print();

    DdpDisplay ddp_display;

    const uint32_t kActivePorts = (argc == 3 ? atoi(argv[2]) : 2);

    ddp_display.SetCount(256, 3, kActivePorts);

    DmxMonitor monitor;

    json::DmxMonitorParams monitor_params;
    monitor_params.Load();
    monitor_params.Set();

    ddp_display.SetOutput(&monitor);

    auto& rdm_device = RdmDevice::Get();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
    rdm_device.Init();
    rdm_device.Print();

    RDMNetDevice llrp_only_device;
    llrp_only_device.Print();

    ddp_display.Print();

    RemoteConfig remote_config(remoteconfig::Output::MONITOR, kActivePorts);

    ddp_display.Start();

    while (keep_running)
    {
        network::Run();
        hal::Run();
    }

    return 0;
}
