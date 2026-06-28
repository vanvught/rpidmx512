/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <signal.h>

#include "board.h"
#include "network.h"
#include "displayudf.h"
#include "json/displayudfparams.h"
#include "json/dmxsendparams.h"
#include "json/dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "artnetrdmresponder.h"
#include "json/dmxnodeparams.h"
#include "dmxmonitor.h"
#include "json/dmxmonitorparams.h"
#include "rdmpersonality.h"
#include "configstore.h"
#include "remoteconfig.h"
#if defined(NODE_SHOWFILE)
#include "showfile.h"
#endif
#include "firmware/firmwareversion.h"
#include "software_version.h"

static bool keep_running = true;

void IntHandler(int) {
    keep_running = false;
}

int main(int argc, char** argv) { // NOLINT
    struct sigaction act;
    act.sa_handler = IntHandler;
    sigaction(SIGINT, &act, nullptr);
    board::Init();
    DisplayUdf display;
    ConfigStore config_store;
    Network nw(argc, argv);
    FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);

    board::Print();
    fw.Print();
    nw.Print();

    DmxMonitor monitor;

    json::DmxMonitorParams monitor_params;
    monitor_params.Load();
    monitor_params.Set();

    RdmPersonality* personalities[1] = {new RdmPersonality("Real-time DMX Monitor", &monitor)};

    ArtNetRdmResponder rdm_responder(personalities, 1);
    rdm_responder.Print();

    DmxNodeNode dmx_node_node;
    dmx_node_node.SetOutput(&monitor);
    dmx_node_node.SetRdmResponder(&rdm_responder, true);
    dmx_node_node.SetRdm(0, true);

    dmx_node_node.Print();

#if defined(NODE_SHOWFILE)
    ShowFile showfile;
    showfile.Print();
#endif

    display.SetTitle("Art-Net 4 %s", dmx_node_node.GetRdm() ? "RDM" : "DMX/RDM");
    display.Set(2, displayudf::Labels::kIp);
    display.Set(3, displayudf::Labels::kVersion);
    display.Set(4, displayudf::Labels::kHostname);
    display.Set(5, displayudf::Labels::kBoardname);
    display.Set(6, displayudf::Labels::kIp);

    json::DisplayUdfParams displayudf_params;
    displayudf_params.Load();
    displayudf_params.SetAndShow();

    RemoteConfig remote_config(remoteconfig::Output::MONITOR, dmx_node_node.GetActiveOutputPorts());

    dmx_node_node.Start();

    while (keep_running) {
        network::Run();
        dmx_node_node.Run();
#if defined(NODE_SHOWFILE)
        showfile.Run();
#endif
        board::Run();
    }

    return 0;
}
