/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdlib>
#include <cctype>
#include <signal.h>

#include "hardware.h"
#include "network.h"

#include "display.h"

#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"
#include "artnetrdmresponder.h"

#include "dmxnodeparams.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#include "configstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "firmwareversion.h"
#include "software_version.h"

static bool keepRunning = true;

void intHandler(int) {
    keepRunning = false;
}

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, nullptr);
	Hardware hw;
	Display display;
	ConfigStore configStore;
	Network nw(argc, argv);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	hal::print();
	fw.Print();
	nw.Print();

	DMXMonitor monitor;

	DMXMonitorParams monitorParams;
	monitorParams.Load();

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality("Real-time DMX Monitor", &monitor)};

	ArtNetRdmResponder RdmResponder(pRDMPersonalities, 1);
	RdmResponder.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&RdmResponder);

	RdmResponder.Print();

	DmxNodeNode dmxNodeNode;
	dmxNodeNode.SetOutput(&monitor);
	dmxNodeNode.SetRdmUID(RdmResponder.GetUID());
	dmxNodeNode.SetRdmResponder(&RdmResponder, true);
	dmxNodeNode.SetRdm(0, true);

	dmxNodeNode.Print();

#if defined (NODE_SHOWFILE)
	ShowFile showFile;

	ShowFileParams showFileParams;
	showFileParams.Load();
	showFileParams.Set();

	if (showFile.IsAutoStart()) {
		showFile.Play();
	}

	showFile.Print();
#endif

	RemoteConfig remoteConfig(remoteconfig::NodeType::ARTNET, remoteconfig::Output::MONITOR, dmxNodeNode.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	dmxNodeNode.Start();

	while (keepRunning) {
		nw.Run();
		dmxNodeNode.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		hal::run();
	}

	return 0;
}
