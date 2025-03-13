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

#include <cstdint>
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "display.h"

#include "net/apps/mdns.h"

#include "oscclient.h"
#include "oscclientparams.h"
#include "oscclientmsgconst.h"
#include "oscclientled.h"

#include "buttonsset.h"
#include "buttonsgpio.h"
#include "buttonsmcp.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"


#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

namespace hal {
void reboot_handler() {
}
}  // namespace hal

int main() {
	Hardware hw;
	Display display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("OSC Client");

	OscClient oscClient;
	
	OscClientParams oscClientParams;
	oscClientParams.Load();
	oscClientParams.Set();

	mdns_service_record_add(nullptr, mdns::Services::OSC, "type=client", oscClient.GetPortIncoming());

	display.TextStatus(OscClientMsgConst::PARAMS, CONSOLE_YELLOW);

	oscClient.Print();

	ButtonsSet *pButtonsSet;

	auto *pButtonsMcp = new ButtonsMcp(&oscClient);
	assert(pButtonsMcp != nullptr);

	if (pButtonsMcp->Start()) {
		pButtonsSet = static_cast<ButtonsSet*>(pButtonsMcp);
		oscClient.SetLedHandler(pButtonsMcp);
	} else {
		delete pButtonsMcp;

		auto *pButtonsGpio = new ButtonsGpio(&oscClient);
		assert(pButtonsGpio != nullptr);

		pButtonsGpio->Start();

		pButtonsSet = static_cast<ButtonsSet*>(pButtonsGpio);
		oscClient.SetLedHandler(pButtonsGpio);
	}

	RemoteConfig remoteConfig(remoteconfig::NodeType::OSC_CLIENT, remoteconfig::Output::OSC, pButtonsSet->GetButtonsCount());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	for (uint32_t i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	display.Write(1, "Eth OSC Client");
	display.Printf(2, "%s.local", nw.GetHostName());
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "S : " IPSTR, IP2STR(oscClient.GetServerIP()));
	display.Printf(5, "O : %d", oscClient.GetPortOutgoing());
	display.Printf(6, "I : %d", oscClient.GetPortIncoming());

	display.TextStatus(OscClientMsgConst::START, CONSOLE_YELLOW);

	oscClient.Start();

	display.TextStatus(OscClientMsgConst::STARTED, CONSOLE_GREEN);

	hal::statusled_set_mode(hal::StatusLedMode::NORMAL);
	hal::watchdog_init();

	for (;;) {
		hal::watchdog_feed();
		nw.Run();
		oscClient.Run();
		pButtonsSet->Run();
		display.Run();
		hal::run();
	}
}
