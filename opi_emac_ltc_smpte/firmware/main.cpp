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

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "net/apps/mdns.h"

#include "display.h"

#include "ltcparams.h"
#include "ltcdisplayparams.h"
#include "ltcdisplayrgb.h"
#include "ltcdisplaymax7219.h"
#include "ltcetc.h"
#include "ltcetcparams.h"

#include "artnetnode.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"

#include "artnetconst.h"

#include "midi.h"
#include "net/rtpmidi.h"
#include "midiparams.h"

#include "tcnet.h"
#include "tcnetparams.h"
#include "tcnettimecode.h"
#include "tcnetdisplay.h"

#include "ntpserver.h"

#include "mcpbuttons.h"
#include "ltcoscserver.h"

#include "ltcsourceconst.h"
#include "ltcsource.h"
#include "ltcsender.h"

#include "arm/artnetreader.h"
#include "arm/ltcreader.h"
#include "arm/midireader.h"
#include "arm/tcnetreader.h"
#include "arm/ltcgenerator.h"
#include "arm/rtpmidireader.h"
#include "arm/systimereader.h"
#include "arm/ltcetcreader.h"
#include "arm/ltcoutputs.h"
#include "arm/ltcmidisystemrealtime.h"

#include "flashcodeinstall.h"

#include "configstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmnetllrponly.h"
# include "rdm_e120.h"
# include "factorydefaults.h"
#endif

#include "net/apps/ntp_client.h"
#include "gpstimeclient.h"
#include "gpsparams.h"

#include "firmwareversion.h"
#include "software_version.h"

#if defined(ENABLE_SHELL)
# include "shell/shell.h"
#endif

#include "net/protocol/ntp.h"

static ltc::Source ltcSource;

namespace ntpclient {
void display_status(const ::ntp::Status status) {
	if (ltcSource != ltc::Source::SYSTIME) {
		return;
	}

	switch (status) {
	case ::ntp::Status::STOPPED:
		Display::Get()->TextStatus("No NTP Client");
		break;
	case ::ntp::Status::IDLE:
		LtcOutputs::Get()->ResetTimeCodeTypePrevious();
		Display::Get()->TextStatus("NTP Client");
		break;
	case ::ntp::Status::LOCKED:
		Display::Get()->TextStatus("NTP Client LOCKED");
		break;
	case ::ntp::Status::FAILED:
		Display::Get()->TextStatus("Error: NTP");
		break;
	default:
		break;
	}
}
}  // namespace ntpclient

namespace hal {
void reboot_handler() {
	switch (ltcSource) {
	case ::ltc::Source::TCNET:
		TCNet::Get()->Stop();
		break;
	default:
		break;
	}

//	if (!ltc::g_DisabledOutputs.bMax7219) {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219)) {
		LtcDisplayMax7219::Get()->Init(2); // TODO WriteChar
	}
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
//	if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
		LtcDisplayRgb::Get()->WriteChar('-');
	}
#endif
	if (!RemoteConfig::Get()->IsReboot()) {
		Display::Get()->SetSleep(false);
		Display::Get()->Cls();
		Display::Get()->TextStatus("Rebooting ...");
	}
}
}  // namespace hal

#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
extern "C" {
void h3_cpu_off(uint32_t);
}
#endif

void static StaticCallbackFunction([[maybe_unused]] const struct artnet::TimeCode *pTimeCode) {}

int main() {
	Hardware hw;
	Display display(4);
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("LTC SMPTE");

#if defined(ENABLE_SHELL)
	Shell shell;
#endif

	display.ClearLine(1);
	display.ClearLine(2);

	LtcParams ltcParams;

	struct ltc::TimeCode tStartTimeCode;
	struct ltc::TimeCode tStopTimeCode;

	ltcParams.Load();
	ltcParams.Set(&tStartTimeCode, &tStopTimeCode);

	LtcReader ltcReader;
	MidiReader midiReader;
	ArtNetReader artnetReader;
	TCNetReader tcnetReader;
	RtpMidiReader rtpMidiReader;
	SystimeReader sysTimeReader(ltcParams.GetFps(), ltcParams.GetUtcOffset());
	LtcEtcReader ltcEtcReader;

	ltcSource = ltcParams.GetSource();

	LtcDisplayParams ltcDisplayParams;

	ltcDisplayParams.Load();
	display.SetContrast(ltcDisplayParams.GetOledIntensity());

	LtcDisplayMax7219 ltcDdisplayMax7219(ltcDisplayParams.GetMax7219Type());
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	LtcDisplayRgb ltcDisplayRgb(ltcParams.IsRgbPanelEnabled() ? ltcdisplayrgb::Type::RGBPANEL : ltcdisplayrgb::Type::WS28XX, ltcDisplayParams.GetWS28xxDisplayType());
#endif
	/**
	 * Select the source using buttons/rotary
	 */

	const auto IsAutoStart = ((ltcSource == ltc::Source::SYSTIME) && ltcParams.IsAutoStart());

	McpButtons sourceSelect(ltcSource, ltcParams.IsAltFunction(), ltcParams.GetSkipSeconds(), !ltcDisplayParams.IsRotaryFullStep());

	if (sourceSelect.Check() && !IsAutoStart) {
		while (sourceSelect.Wait(ltcSource, tStartTimeCode, tStopTimeCode)) {
			nw.Run();
		}
	}

	/**
	 * From here work with source selection
	 */

#if defined(ENABLE_SHELL)
	shell.SetSource(ltcSource);
#endif

	LtcOutputs ltcOutputs(ltcSource, ltcParams.IsShowSysTime());

//	if (!ltc::g_DisabledOutputs.bMax7219) {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219)) {
		ltcDdisplayMax7219.Init(ltcDisplayParams.GetMax7219Intensity());
		ltcDdisplayMax7219.Print();
	}

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
		ltcDisplayParams.Set(&ltcDisplayRgb);

//		if (!ltc::g_DisabledOutputs.bRgbPanel) {
		if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
			ltcDisplayRgb.Init();

			char aInfoMessage[8 + 1];
			uint32_t nLength;
			const char *p = ltcDisplayParams.GetInfoMessage(nLength);
			assert(nLength == 8);
			memcpy(aInfoMessage, p, 8);
			aInfoMessage[8] = '\0';
			ltcDisplayRgb.ShowInfo(aInfoMessage);
		} else {
			ltcDisplayParams.Set(&ltcDisplayRgb);
			ltcDisplayRgb.Init(ltcDisplayParams.GetWS28xxLedType());
		}

		ltcDisplayRgb.Print();
	}
#endif

#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
//	if (ltc::g_DisabledOutputs.bRgbPanel) {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
		for (uint32_t nCpuNumber = 1; nCpuNumber < 4; nCpuNumber++) {
			h3_cpu_off(nCpuNumber);
		}
	}
#endif

#if defined (NODE_RDMNET_LLRP_ONLY)
	RDMNetLLRPOnly rdmNetLLRPOnly("LTC SMPTE");

	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	rdmNetLLRPOnly.Init();
	rdmNetLLRPOnly.Print();
#endif

	/**
	 * Art-Net
	 */

	const auto bRunArtNet = ((ltcSource == ltc::Source::ARTNET) || ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET));

	ArtNetNode node;
	node.SetArtTimeCodeCallbackFunction(StaticCallbackFunction);

	if (bRunArtNet) {
		ArtNetParams artnetparams;
		artnetparams.Load();
		artnetparams.Set();

		node.SetShortName(0, "LTC SMPTE Node");

		if (ltcSource == ltc::Source::ARTNET) {
			node.SetArtTimeCodeCallbackFunction(ArtNetReader::StaticCallbackFunction);
		}

		node.SetTimeCodeIp(ltcParams.GetTimecodeIp());

		if (!ltcParams.IsTimeSyncDisabled()) {
			//TODO Send ArtTimeSync
		}
#if defined (NODE_RDMNET_LLRP_ONLY)
		node.SetRdmUID(rdmNetLLRPOnly.GetRDMNetDevice()->GetUID(), true);
#endif
		node.Start();
		node.Print();
	}

	/**
	 * TCNet
	 */

	const auto bRunTCNet = (ltcSource == ltc::Source::TCNET);

	TCNet tcnet;

	if (bRunTCNet) {
		TCNetParams tcnetparams;
		tcnetparams.Load();
		tcnetparams.Set(&tcnet);

		tcnet.SetArtTimeCodeCallbackFunction(TCNetReader::StaticCallbackFunctionHandler);
		tcnet.Start();
		tcnet.Print();
	}

	/**
	 * MIDI
	 */

	Midi midi;

	if ((ltcSource != ltc::Source::MIDI) && ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI)) {
		midi.Init(midi::Direction::OUTPUT);
	}

	if ((ltcSource == ltc::Source::MIDI) || ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI)) {
		midi.Print();
	}

	/**
	 * RTP-MIDI
	 */

	RtpMidi rtpMidi;

	if ((ltcSource == ltc::Source::APPLEMIDI) || ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI)) {
		if (ltcSource == ltc::Source::APPLEMIDI) {
			rtpMidi.SetHandler(&rtpMidiReader);
		}

		rtpMidi.Start();
		rtpMidi.Print();
	}

	/**
	 * ETC
	 */

	LtcEtc ltcEtc;

	if ((ltcSource == ltc::Source::ETC) || ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
		LtcEtcParams ltcEtcParams;

		ltcEtcParams.Load();
		ltcEtcParams.Set();

		if (ltcSource == ltc::Source::ETC) {
			ltcEtc.SetHandler(&ltcEtcReader);
		}

		ltcEtc.Start();
		ltcEtc.Print();
	}

	/**
	 * LTC Sender
	 */

	LtcSender ltcSender(ltcParams.GetVolume());

	if ((ltcSource != ltc::Source::LTC) && ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
		ltcSender.Start();
	}

	/**
	 * The OSC Server is running when enabled AND source = TCNet OR Internal OR System-Time
	 */

	const auto bRunOSCServer = ((ltcSource == ltc::Source::TCNET || ltcSource == ltc::Source::INTERNAL || ltcSource == ltc::Source::SYSTIME) && ltcParams.IsOscEnabled());

	LtcOscServer oscServer;

	if (bRunOSCServer) {
		bool isSet;
		const uint16_t nPort = ltcParams.GetOscPort(isSet);

		if (isSet) {
			oscServer.SetPortIncoming(nPort);
		}

		oscServer.Start();
		oscServer.Print();

		mdns_service_record_add(nullptr, mdns::Services::OSC, "type=server", oscServer.GetPortIncoming());
	}

	/**
	 * The GPS Time client is running when enabled AND source = System-Time AND RGB Panel is disabled
	 * The NTP Client is stopped.
	 */

	GPSParams gpsParams;

	if (ltcSource == ltc::Source::SYSTIME) {
		gpsParams.Load();
	}

	const auto bRunGpsTimeClient = (gpsParams.IsEnabled() && (ltcSource == ltc::Source::SYSTIME) && ltc::Destination::IsDisabled(ltc::Destination::Output::RGBPANEL));
	const auto bGpsStart = bRunGpsTimeClient && ltcParams.IsGpsStart();

	GPSTimeClient gpsTimeClient(gpsParams.GetUtcOffset(), gpsParams.GetModule());

	if (bRunGpsTimeClient) {
		ntp_client_stop(true);
		gpsTimeClient.Start();
		gpsTimeClient.Print();
	}

	/**
	 * When the NTP Server is enabled then the NTP Client is not running (stopped).
	 */

	const auto bRunNtpServer = ltcParams.IsNtpEnabled();

	NtpServer ntpServer(ltcParams.GetYear(), ltcParams.GetMonth(), ltcParams.GetDay());

	if (bRunNtpServer) {
		ntp_client_stop(true);

		ntpServer.SetTimeCode(&tStartTimeCode);
		ntpServer.Start();
		ntpServer.Print();

		mdns_service_record_add(nullptr, mdns::Services::NTP, "type=server");
	}

	/**
	 * LTC Generator
	 */

	LtcGenerator ltcGenerator(&tStartTimeCode, &tStopTimeCode, ltcParams.GetSkipFree(), ltcParams.IsIgnoreStart(), ltcParams.IsIgnoreStop());

	/**
	 * MIDI output System Real Time
	 */

	LtcMidiSystemRealtime ltcMidiSystemRealtime;

	/**
	 * The UDP request handler is running when source is NOT MIDI AND source is NOT RTP-MIDI
	 * AND when MIDI output is NOT disabled OR the RTP-MIDI is NOT disabled.
	 */

	const auto bRunMidiSystemRealtime = (ltcSource != ltc::Source::MIDI) && (ltcSource != ltc::Source::APPLEMIDI) && (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI) || ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI));

	if (bRunMidiSystemRealtime) {
		ltcMidiSystemRealtime.Start();
	}

	/**
	 * Start the reader
	 */

	switch (ltcSource) {
	case ltc::Source::ARTNET:
		artnetReader.Start();
		break;
	case ltc::Source::MIDI:
		midiReader.Start();
		break;
	case ltc::Source::TCNET:
		tcnetReader.Start();
		break;
	case ltc::Source::INTERNAL:
		ltcGenerator.Start();
		ltcGenerator.Print();
		break;
	case ltc::Source::APPLEMIDI:
		rtpMidiReader.Start();
		break;
	case ltc::Source::ETC:
		ltcEtcReader.Start();
		break;
	case ltc::Source::SYSTIME:
		sysTimeReader.Start(ltcParams.IsAutoStart() && !bGpsStart);
		break;
	default:
		ltcReader.Start();
		break;
	}

	RemoteConfig remoteConfig(remoteconfig::Node::LTC, remoteconfig::Output::TIMECODE, 1U + static_cast<uint32_t>(ltcSource));

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	printf("Source : %s\n", LtcSourceConst::NAME[static_cast<uint32_t>(ltcSource)]);

	ltc::source::show(ltcSource, bRunGpsTimeClient);

#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
		ltcDisplayRgb.ShowSource(ltcSource);
	}
#endif

	ltcOutputs.Print();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		// Run the reader
		// Handles MIDI Quarter Frame output messages
		switch (ltcSource) {
		case ltc::Source::LTC:
			ltcReader.Run();
			break;
		case ltc::Source::ARTNET:
			artnetReader.Run();
			break;
		case ltc::Source::MIDI:
			midiReader.Run();
			break;
		case ltc::Source::TCNET:
			tcnetReader.Run();
			break;
		case ltc::Source::INTERNAL:
			ltcGenerator.Run();
			break;
		case ltc::Source::APPLEMIDI:
			rtpMidiReader.Run();
			break;
		case ltc::Source::ETC:
			ltcEtcReader.Run();
			break;
		case ltc::Source::SYSTIME:
			sysTimeReader.Run();
			if (!bRunGpsTimeClient) {
				if (bRunNtpServer) {
					HwClock::Get()->Run(true);
				} else {
					HwClock::Get()->Run(ntp_client_get_status() == ::ntp::Status::FAILED); // No need to check for STOPPED
				}
			} else {
				gpsTimeClient.Run();
				if (bGpsStart) {
					if (gpsTimeClient.GetStatus() == gps::Status::VALID) {
						sysTimeReader.ActionStart();
					} else {
						sysTimeReader.ActionStop();
					}
				}
			}
			break;
		default:
			break;
		}

		if (bRunArtNet) {
			node.Run();
		}

		if (bRunTCNet) {
			tcnet.Run();
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED)) {
			display.Run();
		}

		if (sourceSelect.IsConnected()) {
			sourceSelect.Run();
		}

		hw.Run();
#if defined(ENABLE_SHELL)
		shell.Run();
#endif
	}
}
