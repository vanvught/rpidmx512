/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <mcpbuttonsconst.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "ltcparams.h"
#include "ltcdisplayparams.h"
#include "ltc7segment.h"

#include "artnetnode.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"
#include "timesync.h"

#include "artnetconst.h"
#include "networkconst.h"

#include "ipprog.h"
#include "midi.h"
#include "rtpmidi.h"
#include "midiparams.h"

#include "mdnsservices.h"

#include "tcnet.h"
#include "tcnetparams.h"
#include "tcnettimecode.h"
#include "tcnetdisplay.h"

#include "ntpserver.h"
#include "ntpclient.h"

#include "display.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"

#include "networkhandleroled.h"

#include "mcpbuttons.h"

#include "ltcoscserver.h"

#include "ntpserver.h"

#include "h3/ltc.h"

#include "spiflashinstall.h"

#include "spiflashstore.h"
#include "storeltc.h"
#include "storeltcdisplay.h"
#include "storeartnet.h"
#include "storetcnet.h"
#include "storeremoteconfig.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

// LLRP Only Device
#include "rdmnetllrponly.h"
#include "rdm_e120.h"

// LLRP Handler
#include "factorydefaults.h"

// Reboot handler
#include "reboot.h"

#include "firmwareversion.h"
#include "software_version.h"

// UART0 shell
#if defined ( ENABLE_SHELL )
# include <h3/shell.h>
#endif

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
#if defined ( ENABLE_SHELL)
	Shell shell;
#endif

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreLtc storeLtc;
	LtcParams ltcParams(&storeLtc);

	TLtcDisabledOutputs tLtcDisabledOutputs;
	TLtcTimeCode tStartTimeCode;
	TLtcTimeCode tStopTimeCode;

	if (ltcParams.Load()) {
		ltcParams.CopyDisabledOutputs(&tLtcDisabledOutputs);
		ltcParams.StartTimeCodeCopyTo(&tStartTimeCode);
		ltcParams.StopTimeCodeCopyTo(&tStopTimeCode);
		ltcParams.Dump();
	}

	Ltc7segment leds;

	display.PrintInfo();

	fw.Print("LTC SMPTE");

	hw.SetLed(HARDWARE_LED_ON);

	display.ClearLine(1);
	display.ClearLine(2);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	NetworkHandlerOled networkHandlerOled;

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(&networkHandlerOled);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	NtpClient ntpClient;
	ntpClient.SetNtpClientDisplay(&networkHandlerOled);
	ntpClient.Init();
	ntpClient.Print();

	LtcReader ltcReader(&tLtcDisabledOutputs);
	MidiReader midiReader(&tLtcDisabledOutputs);
	ArtNetReader artnetReader(&tLtcDisabledOutputs);
	TCNetReader tcnetReader(&tLtcDisabledOutputs);
	RtpMidiReader rtpMidiReader(&tLtcDisabledOutputs);
	SystimeReader sysTimeReader(&tLtcDisabledOutputs, ltcParams.GetFps());

	StoreLtcDisplay storeLtcDisplay;
	LtcDisplayParams ltcDisplayParams(&storeLtcDisplay);

	if (ltcDisplayParams.Load()) {
		ltcDisplayParams.Dump();
	}

	LtcDisplayMax7219 ltcDdisplayMax7219(ltcDisplayParams.GetMax7219Type());

	if(!tLtcDisabledOutputs.bMax7219) {
		ltcDdisplayMax7219.Init(ltcDisplayParams.GetMax7219Intensity());
		ltcDdisplayMax7219.Print();
	}

	LtcDisplayWS28xx ltcDisplayWS28xx(ltcDisplayParams.GetWS28xxType());

	if (!tLtcDisabledOutputs.bWS28xx){
		ltcDisplayParams.Set(&ltcDisplayWS28xx);
		ltcDisplayWS28xx.Init(ltcDisplayParams.GetLedType());
		ltcDisplayWS28xx.Print();
	}

	NetworkHandlerOled::Get()->ShowIp();

	ltc::source ltcSource = ltcParams.GetSource();

	/**
	 * Select the source using buttons/rotary
	 */

	const bool IsAutoStart = ((ltcSource == ltc::source::SYSTIME) && ltcParams.IsAutoStart());

	McpButtons sourceSelect(ltcSource, &tLtcDisabledOutputs, ltcParams.IsAltFunction(), ltcParams.GetSkipSeconds());

	if (sourceSelect.Check() && !IsAutoStart) {
		while (sourceSelect.Wait(ltcSource, tStartTimeCode, tStopTimeCode)) {
			nw.Run();
			lb.Run();
		}
	}

	/**
	 * From here work with source selection
	 */

#if defined ( ENABLE_SHELL )
	shell.SetSource(ltcSource);
#endif

	Reboot reboot(ltcSource);
	hw.SetRebootHandler(&reboot);

	Display7Segment::Get()->Status(Display7SegmentMessage::INFO_NONE);

	LtcOutputs ltcOutputs(&tLtcDisabledOutputs, ltcSource, ltcParams.IsShowSysTime());

	/**
	 * Art-Net
	 */

	const bool bRunArtNet = ((ltcSource == ltc::source::ARTNET) || (!tLtcDisabledOutputs.bArtNet));

	ArtNetNode node;
	IpProg ipprog;
	TimeSync timeSync;

	if (bRunArtNet) {
		display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

		ArtNetParams artnetparams(new StoreArtNet);

		if (artnetparams.Load()) {
			artnetparams.Set(&node);
			artnetparams.Dump();
		}

		node.SetArtNetStore(StoreArtNet::Get());
		node.SetShortName("LTC SMPTE Node");
		node.SetIpProgHandler(&ipprog);

		if (!ltcParams.IsTimeSyncDisabled()) {
			node.SetTimeSyncHandler(&timeSync);
		}

		display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

		node.Start();
		node.Print();

		display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);
	}

	/**
	 * TCNet
	 */

	const bool bRunTCNet = (ltcSource == ltc::source::TCNET);

	TCNet tcnet(TCNET_TYPE_SLAVE);
	StoreTCNet storetcnet;

	if (bRunTCNet) {
		TCNetParams tcnetparams(&storetcnet);

		if (tcnetparams.Load()) {
			tcnetparams.Set(&tcnet);
			tcnetparams.Dump();
		}

		tcnet.Start();
		tcnet.Print();
	}

	/**
	 * MIDI
	 */

	Midi midi;

	if (ltcSource != ltc::source::MIDI) {
		midi.Init(MIDI_DIRECTION_OUTPUT);
	}

	midi.Print();

	/**
	 * RTP-MIDI
	 */

	const bool bRunRtpMidi = ((ltcSource == ltc::source::APPLEMIDI) || (!tLtcDisabledOutputs.bRtpMidi));

	RtpMidi rtpMidi;

	if (bRunRtpMidi) {
		rtpMidi.Start();
		rtpMidi.AddServiceRecord(0, MDNS_SERVICE_CONFIG, 0x2905);
		rtpMidi.Print();
	}

	/**
	 * LTC Sender
	 */

	LtcSender ltcSender;

	if ((ltcSource != ltc::source::LTC) && (!tLtcDisabledOutputs.bLtc)) {
		ltcSender.Start();
	}

	/**
	 * The OSC Server is running when enabled AND source = TCNet OR Internal OR System-Time
	 */

	const bool bRunOSCServer = ((ltcSource == ltc::source::TCNET || ltcSource == ltc::source::INTERNAL || ltcSource == ltc::source::SYSTIME) && ltcParams.IsOscEnabled());

	LtcOscServer oscServer;

	if (bRunOSCServer) {
		bool isSet;
		const uint16_t nPort = ltcParams.GetOscPort(isSet);

		if (isSet) {
			oscServer.SetPortIncoming(nPort);
		}

		oscServer.Start();
		oscServer.Print();
		rtpMidi.AddServiceRecord(0, MDNS_SERVICE_OSC, oscServer.GetPortIncoming(), "type=server");
	}

	/**
	 * NTP Server is running when the NTP Client is not running (stopped)
	 */

	const bool bRunNtpServer = ltcParams.IsNtpEnabled() && (ntpClient.GetStatus() == NtpClientStatus::STOPPED);

	NtpServer ntpServer(ltcParams.GetYear(), ltcParams.GetMonth(), ltcParams.GetDay());

	if (bRunNtpServer) {
		ntpServer.SetTimeCode(&tStartTimeCode);
		ntpServer.Start();
		ntpServer.Print();
		rtpMidi.AddServiceRecord(0, MDNS_SERVICE_NTP, NTP_UDP_PORT, "type=server");
	}

	/**
	 * LTC Generator
	 */

	LtcGenerator ltcGenerator(&tStartTimeCode, &tStopTimeCode, &tLtcDisabledOutputs, ltcParams.GetSkipFree());

	/**
	 * Start the reader
	 */

	switch (ltcSource) {
	case ltc::source::ARTNET:
		node.SetTimeCodeHandler(&artnetReader);
		artnetReader.Start();
		break;
	case ltc::source::MIDI:
		midiReader.Start();
		break;
	case ltc::source::TCNET:
		tcnet.SetTimeCodeHandler(&tcnetReader);
		tcnetReader.Start();
		break;
	case ltc::source::INTERNAL:
		ltcGenerator.Start();
		ltcGenerator.Print();
		break;
	case ltc::source::APPLEMIDI:
		rtpMidi.SetHandler(&rtpMidiReader);
		rtpMidiReader.Start();
		break;
	case ltc::source::SYSTIME:
		sysTimeReader.Start(ltcParams.IsAutoStart());
		break;
	default:
		ltcReader.Start();
		break;
	}

	RDMNetLLRPOnly rdmNetLLRPOnly("LTC SMPTE");

//	rdmNetLLRPOnly.GetRDMNetDevice()->SetRDMFactoryDefaults(new FactoryDefaults);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	rdmNetLLRPOnly.Init();
	rdmNetLLRPOnly.Print();
	rdmNetLLRPOnly.Start();

	RemoteConfig remoteConfig(REMOTE_CONFIG_LTC, REMOTE_CONFIG_MODE_TIMECODE, 1 + ltcSource);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	printf("Source : %s\n", McpButtonsConst::SOURCE[ltcSource]);

	// OLED display

	display.ClearLine(4);
	display.PutString(McpButtonsConst::SOURCE[ltcSource]);

	if (ltcSource == ltc::source::TCNET) {
		TCNetDisplay::Show();
	}

	ltcOutputs.Print();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();

		// Run the reader
		switch (ltcSource) {
		case ltc::source::LTC:
			ltcReader.Run();
			break;
		case ltc::source::ARTNET:
			artnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::source::MIDI:
			midiReader.Run();
			break;
		case ltc::source::TCNET:
			tcnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::source::INTERNAL:
			ltcGenerator.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::source::APPLEMIDI:
			rtpMidiReader.Run();	// Handles status led
			break;
		case ltc::source::SYSTIME:
			sysTimeReader.Run();
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

		if (bRunRtpMidi) {
			rtpMidi.Run();
		}

		if (bRunOSCServer) {
			oscServer.Run();
		}

		if (bRunNtpServer) {
			ntpServer.Run();
		} else {
			ntpClient.Run();
		}

		if (tLtcDisabledOutputs.bDisplay) {
			display.Run();
		}

		if (!tLtcDisabledOutputs.bWS28xx){
			ltcDisplayWS28xx.Run();
		}

		if (sourceSelect.IsConnected()) {
			sourceSelect.Run();
		}
		//
		rdmNetLLRPOnly.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		//
#if defined ( ENABLE_SHELL )
		shell.Run();
#endif
	}
}

}
