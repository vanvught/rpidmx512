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

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "console.h"

#include "ltcparams.h"
#include "ltcdisplayparams.h"
#include "ltc7segment.h"

#include "artnetnode.h"
#include "artnetparams.h"
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

#include "oscserver.h"

#include "display.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"

#include "networkhandleroled.h"

#include "sourceselect.h"
#include "sourceselectconst.h"

#include "ntpserver.h"

#include "h3/ltc.h"

#include "spiflashinstall.h"

#include "spiflashstore.h"
#include "storeltc.h"
#include "storeltcdisplay.h"
#include "storetcnet.h"
#include "storeremoteconfig.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "reboot.h"

#include "firmwareversion.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreLtc storeLtc;
	LtcParams ltcParams(&storeLtc);

	struct TLtcDisabledOutputs tLtcDisabledOutputs;
	struct TLtcTimeCode tStartTimeCode;
	struct TLtcTimeCode tStopTimeCode;

	if (ltcParams.Load()) {
		ltcParams.CopyDisabledOutputs(&tLtcDisabledOutputs);
		ltcParams.StartTimeCodeCopyTo(&tStartTimeCode);
		ltcParams.StopTimeCodeCopyTo(&tStopTimeCode);
		ltcParams.Dump();
	}

	Ltc7segment leds;

	fw.Print();

	hw.SetLed(HARDWARE_LED_ON);

	display.ClearLine(1);
	display.ClearLine(2);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init(spiFlashStore.GetStoreNetwork());
	nw.SetNetworkStore(spiFlashStore.GetStoreNetwork());
	nw.SetNetworkDisplay(new NetworkHandlerOled);
	nw.Print();

	NtpClient ntpClient;
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

	display.ClearLine(3);
	display.Printf(3, IPSTR "/%d %c", IP2STR(nw.GetIp()), static_cast<int>(nw.GetNetmaskCIDR()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');

	TLtcReaderSource source = ltcParams.GetSource();

	/**
	 * Select the source using buttons/rotary
	 */

	const bool IsAutoStart = ((source == LTC_READER_SOURCE_SYSTIME) && ltcParams.IsAutoStart());

	SourceSelect sourceSelect(source, &tLtcDisabledOutputs);

	if (sourceSelect.Check() && !IsAutoStart) {
		while (sourceSelect.Wait(source)) {
			nw.Run();
			lb.Run();
		}
	}

	/**
	 * From here work with source selection
	 */

	Reboot reboot(source);
	hw.SetRebootHandler(&reboot);

	display.Status(DISPLAY_7SEGMENT_MSG_INFO_NONE);

	LtcOutputs ltcOutputs(&tLtcDisabledOutputs, source, ltcParams.IsShowSysTime());

	/**
	 * Art-Net
	 */

	const bool bRunArtNet = ((source == LTC_READER_SOURCE_ARTNET) || (!tLtcDisabledOutputs.bArtNet));

	ArtNetNode node;
	IpProg ipprog;
	TimeSync timeSync;

	if (bRunArtNet) {
		console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_PARAMS);
		display.TextStatus(ArtNetConst::MSG_NODE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_NODE_PARMAMS);

		ArtNetParams artnetparams(spiFlashStore.GetStoreArtNet());

		if (artnetparams.Load()) {
			artnetparams.Set(&node);
			artnetparams.Dump();
		}

		node.SetArtNetStore(spiFlashStore.GetStoreArtNet());
		node.SetShortName("LTC SMPTE Node");
		node.SetIpProgHandler(&ipprog);

		if (!ltcParams.IsTimeSyncDisabled()) {
			node.SetTimeSyncHandler(&timeSync);
		}

		console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_START);
		display.TextStatus(ArtNetConst::MSG_NODE_START, DISPLAY_7SEGMENT_MSG_INFO_NODE_START);

		node.Start();
		node.Print();

		console_status(CONSOLE_GREEN, ArtNetConst::MSG_NODE_STARTED);
		display.TextStatus(ArtNetConst::MSG_NODE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_NODE_STARTED);
	}

	/**
	 * TCNet
	 */

	const bool bRunTCNet = (source == LTC_READER_SOURCE_TCNET);

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

	if (source != LTC_READER_SOURCE_MIDI) {
		midi.Init(MIDI_DIRECTION_OUTPUT);
	}

	midi.Print();

	/**
	 * RTP-MIDI
	 */

	const bool bRunRtpMidi = ((source == LTC_READER_SOURCE_APPLEMIDI) || (!tLtcDisabledOutputs.bRtpMidi));

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

	if ((source != LTC_READER_SOURCE_LTC) && (!tLtcDisabledOutputs.bLtc)) {
		ltcSender.Start();
	}

	/**
	 * The OSC Server is running when enabled AND source = TCNet OR Internal OR System-Time
	 */

	const bool bRunOSCServer = ((source == LTC_READER_SOURCE_TCNET || source == LTC_READER_SOURCE_INTERNAL || source == LTC_READER_SOURCE_SYSTIME) && ltcParams.IsOscEnabled());

	OSCServer oscServer;

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

	const bool bRunNtpServer = ltcParams.IsNtpEnabled() && (ntpClient.GetStatus() == NTP_CLIENT_STATUS_STOPPED);

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

	LtcGenerator ltcGenerator(&tStartTimeCode, &tStopTimeCode, &tLtcDisabledOutputs);

	/**
	 * Start the reader
	 */

	switch (source) {
	case LTC_READER_SOURCE_ARTNET:
		node.SetTimeCodeHandler(&artnetReader);
		artnetReader.Start();
		break;
	case LTC_READER_SOURCE_MIDI:
		midiReader.Start();
		break;
	case LTC_READER_SOURCE_TCNET:
		tcnet.SetTimeCodeHandler(&tcnetReader);
		tcnetReader.Start();
		break;
	case LTC_READER_SOURCE_INTERNAL:
		ltcGenerator.Start();
		ltcGenerator.Print();
		break;
	case LTC_READER_SOURCE_APPLEMIDI:
		rtpMidi.SetHandler(&rtpMidiReader);
		rtpMidiReader.Start();
		break;
	case LTC_READER_SOURCE_SYSTIME:
		sysTimeReader.Start(ltcParams.IsAutoStart());
		break;
	default:
		ltcReader.Start();
		break;
	}

	RemoteConfig remoteConfig(REMOTE_CONFIG_LTC, REMOTE_CONFIG_MODE_TIMECODE, 1 + source);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	printf("Source : %s\n", SourceSelectConst::SOURCE[source]);

	// OLED display

	display.ClearLine(4);
	display.PutString(SourceSelectConst::SOURCE[source]);

	if (source == LTC_READER_SOURCE_TCNET) {
		TCNetDisplay::Show();
	}

	ltcOutputs.Print();

	printf("Display : %d (%d,%d)\n", display.GetDetectedType(), display.getCols(), display.getRows());

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();

		// Run the reader
		switch (source) {
		case LTC_READER_SOURCE_LTC:
			ltcReader.Run();
			break;
		case LTC_READER_SOURCE_ARTNET:
			artnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_MIDI:
			midiReader.Run();
			break;
		case LTC_READER_SOURCE_TCNET:
			tcnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_INTERNAL:
			ltcGenerator.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_APPLEMIDI:
			rtpMidiReader.Run();	// Handles status led
			break;
		case LTC_READER_SOURCE_SYSTIME:
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

		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
	}
}

}
