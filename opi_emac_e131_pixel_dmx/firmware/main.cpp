/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "networkconst.h"

#include "mdns.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"

#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "pixeldmxparams.h"
#include "ws28xxdmx.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "dmxconfigudp.h"

#include "lightset4with4.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmdeviceparams.h"
# include "rdmnetdevice.h"
# include "rdmnetconst.h"
# include "rdmpersonality.h"
# include "rdm_e120.h"
# include "factorydefaults.h"
#endif

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

namespace e131bridge {
namespace configstore {
uint32_t DMXPORT_OFFSET = 4;
}  // namespace configstore
}  // namespace e131bridge

void Hardware::RebootHandler() {
	WS28xx::Get()->Blackout();
	Dmx::Get()->Blackout();
	E131Bridge::Get()->Stop();
}

void main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	Network nw;
	MDNS mDns;
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("sACN E1.31 Pixel controller {1x 4 Universes} / DMX");
	nw.Print();

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	E131Bridge bridge;

	E131Params e131params;
	e131params.Load();
	e131params.Set();

	// LightSet A - Pixel - 4 Universes

	PixelDmxConfiguration pixelDmxConfiguration;

	PixelDmxParams pixelDmxParams;
	pixelDmxParams.Load();
	pixelDmxParams.Set(&pixelDmxConfiguration);

	WS28xxDmx pixelDmx(pixelDmxConfiguration);

	auto isPixelUniverseSet = false;
	const auto nStartPixelUniverse = pixelDmxParams.GetStartUniversePort(0, isPixelUniverseSet);

	if (isPixelUniverseSet) {
		const auto nUniverses = pixelDmx.GetUniverses();

		for (uint32_t nPortIndex = 0; nPortIndex < nUniverses; nPortIndex++) {
			bridge.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nStartPixelUniverse + nPortIndex));
		}
	}

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(pixelDmxParams.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern, 1);

	// LightSet B - DMX - 1 Universe

	auto isDmxUniverseSet = false;
	const auto portDirection = e131params.GetDirection(0);

	if (portDirection == lightset::PortDir::OUTPUT) {
		bridge.SetUniverse(e131bridge::configstore::DMXPORT_OFFSET, lightset::PortDir::OUTPUT, e131params.GetUniverse(0, isDmxUniverseSet));
	}

	Dmx dmx;

	DmxParams dmxparams;
	dmxparams.Load();
	dmxparams.Set(&dmx);

	uint16_t nUniverse;

	if (bridge.GetUniverse(e131bridge::configstore::DMXPORT_OFFSET, nUniverse, lightset::PortDir::OUTPUT)) {
		dmx.SetPortDirection(0, dmx::PortDirection::OUTP, false);
	} else {
		dmx.SetPortDirection(0, dmx::PortDirection::INP, false);
	}

	DmxSend dmxSend;
	dmxSend.Print();

	display.SetDmxInfo(displayudf::dmx::PortDir::OUTPUT, isDmxUniverseSet ? 1 : 0);

	// LightSet 4with4

	LightSet4with4 lightSet((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE) ? nullptr : &pixelDmx, isDmxUniverseSet ? &dmxSend : nullptr);
	lightSet.Print();

	bridge.SetOutput(&lightSet);
	bridge.Print();

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_CONFIG, Display7SegmentMessage::INFO_RDMNET_CONFIG, CONSOLE_YELLOW);

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "sACN Pixel %d-%s:%d DMX %d", isPixelUniverseSet, PixelType::GetType(WS28xx::Get()->GetType()), WS28xx::Get()->GetCount(), isDmxUniverseSet);

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, "Orange Pi Zero Pixel-DMX");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_LED);
	llrpOnlyDevice.Init();

	RDMDeviceParams rdmDeviceParams;

	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&llrpOnlyDevice);

	llrpOnlyDevice.Print();
#endif

#if defined (NODE_SHOWFILE)
	ShowFile showFile;

	ShowFileParams showFileParams;
	showFileParams.Load();
	showFileParams.Set();

	if (showFile.IsAutoStart()) {
		showFile.Start();
	}

	showFile.Print();
#endif

	display.SetTitle("sACN E1.31 Pixel 1 - DMX");
	display.Set(2, displayudf::Labels::VERSION);
	display.Set(3, displayudf::Labels::HOSTNAME);
	display.Set(4, displayudf::Labels::IP);
	display.Set(5, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(6, displayudf::Labels::DMX_DIRECTION);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show(&bridge);

	display.Printf(7, "%s:%d G%d %s",
		PixelType::GetType(pixelDmxConfiguration.GetType()),
		pixelDmxConfiguration.GetCount(),
		pixelDmxConfiguration.GetGroupingCount(),
		PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	if (nTestPattern != pixelpatterns::Pattern::NONE) {
		display.ClearLine(6);
		display.Printf(6, "%s:%u", PixelPatterns::GetName(nTestPattern), static_cast<uint32_t>(nTestPattern));
	}

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::PIXEL, bridge.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	mDns.Print();

	display.TextStatus(E131MsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	bridge.Start();

	display.TextStatus(E131MsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		remoteConfig.Run();
#if defined (NODE_RDMNET_LLRP_ONLY)
		llrpOnlyDevice.Run();
#endif
		configStore.Flash();
		if (__builtin_expect((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE), 0)) {
			pixelTestPattern.Run();
		}
		mDns.Run();
		display.Run();
		hw.Run();
	}
}
