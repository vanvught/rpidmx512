/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal.h"
#include "network.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "dmxparams.h"
#include "dmxsend.h"

#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"

#include "rdmdeviceparams.h"
#include "artnetrdmcontroller.h"

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashcodeinstall.h"
#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace hal {
void reboot_handler() {
	Dmx::Get()->Blackout();
	ArtNetNode::Get()->Stop();
}
}  // namespace hal

static constexpr uint32_t PORT_INDEX = 0;

int main() {
	hal_init();
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("Art-Net 4, Universes: " STR(DMXNODE_PORTS) " DMX/RDM");

	Dmx dmx;

	DmxParams dmxparams;
	dmxparams.Load();
	dmxparams.Set(&dmx);

	DmxSend dmxSend;
	dmxSend.Print();

	DmxNodeNode dmxNodeNode;
	dmxNodeNode.SetOutput(&dmxSend);
	
	const auto portDirection = (dmxNodeNode.GetPortDirection(PORT_INDEX) == dmxnode::PortDirection::OUTPUT ? dmx::PortDirection::OUTP : dmx::PortDirection::INP);
	dmx.SetPortDirection(PORT_INDEX, portDirection , false);

	ArtNetRdmController artNetRdmController;

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&artNetRdmController);

	artNetRdmController.Init();
	artNetRdmController.Print();

	const auto isRdmEnabled = dmxNodeNode.GetRdm();

	dmxNodeNode.SetRdmController(&artNetRdmController, isRdmEnabled);

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

	dmxNodeNode.Print();

	const auto nActivePorts = dmxNodeNode.GetActiveInputPorts() + dmxNodeNode.GetActiveOutputPorts();

	display.SetTitle("Art-Net 4 %s", portDirection == dmx::PortDirection::INP ? "DMX Input" : (isRdmEnabled ? "RDM" : "DMX Output"));
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::VERSION);
	display.Set(4, displayudf::Labels::HOSTNAME);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_A);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	RemoteConfig remoteConfig(remoteconfig::NodeType::ARTNET, isRdmEnabled ? remoteconfig::Output::RDM : remoteconfig::Output::DMX, nActivePorts);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	display.TextStatus(DmxNodeMsgConst::START, CONSOLE_YELLOW);

	dmxNodeNode.Start();

	display.TextStatus(DmxNodeMsgConst::STARTED, CONSOLE_GREEN);

	hal::watchdog_init();

	for (;;) {
		hal::watchdog_feed();
		nw.Run();
		dmxNodeNode.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		display.Run();
		hal::run();
	}
}
