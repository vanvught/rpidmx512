/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarebaremetal.h"
#include "networkh3emac.h"
#include "ledblinkbaremetal.h"

#include "console.h"

#include "ltcparams.h"
#include "ltcleds.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "artnetconst.h"
#include "networkconst.h"

#include "ipprog.h"
#include "timesync.h"

#include "midi.h"
#include "midiparams.h"

#include "tcnet.h"
#include "tcnetparams.h"
#include "tcnettimecode.h"

#include "ntpserver.h"

#include "display.h"
#include "displaymax7219.h"

#include "ntpserver.h"

#include "h3/artnetreader.h"
#include "h3/ltcreader.h"
#include "h3/ltcsender.h"
#include "h3/midireader.h"
#include "h3/tcnetreader.h"
#include "h3/ltcgenerator.h"

#include "spiflashinstall.h"

#include "spiflashstore.h"
#include "storeltc.h"
#include "storetcnet.h"
#include "storeremoteconfig.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "firmwareversion.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;

	SpiFlashStore spiFlashStore;

	StoreLtc storeLtc;

	LtcParams ltcParams((LtcParamsStore *)&storeLtc);

	struct TLtcDisabledOutputs tLtcDisabledOutputs;
	memset(&tLtcDisabledOutputs, 0, sizeof(struct TLtcDisabledOutputs));

	struct TLtcTimeCode tStartTimeCode;
	memset(&tStartTimeCode, 0, sizeof(struct TLtcTimeCode));

	struct TLtcTimeCode tStopTimeCode;
	memset(&tStopTimeCode, 0, sizeof(struct TLtcTimeCode));

	if (ltcParams.Load()) {
		ltcParams.Dump();
		ltcParams.CopyDisabledOutputs(&tLtcDisabledOutputs);
		ltcParams.StartTimeCodeCopyTo(&tStartTimeCode);
		ltcParams.StopTimeCodeCopyTo(&tStopTimeCode);
	}

	const bool bEnableNtp = ltcParams.IsNtpEnabled();
	tLtcDisabledOutputs.bNtp = !bEnableNtp;

	LtcLeds leds;

	Midi midi;
	ArtNetNode node;
	TCNet tcnet;
	LtcSender ltcSender;

	LtcReader ltcReader(&node, &tLtcDisabledOutputs);
	MidiReader midiReader(&node, &tLtcDisabledOutputs);
	ArtNetReader artnetReader(&tLtcDisabledOutputs);
	TCNetReader tcnetReader(&node, &tLtcDisabledOutputs);

	LtcGenerator ltcGenerator(&node, &tStartTimeCode, &tStopTimeCode, &tLtcDisabledOutputs);

	NtpServer ntpServer(ltcParams.GetYear(), ltcParams.GetMonth(), ltcParams.GetDay());

	fw.Print();

	hw.SetLed(HARDWARE_LED_ON);

	display.ClearLine(1);
	display.ClearLine(2);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_PARAMS);
	display.TextStatus(ArtNetConst::MSG_NODE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_NODE_PARMAMS);

	node.SetShortName("LTC Node");

	ArtNetParams artnetparams((ArtNetParamsStore *)spiFlashStore.GetStoreArtNet());

	if (artnetparams.Load()) {
		artnetparams.Set(&node);
		artnetparams.Dump();
	}

	IpProg ipprog;
	node.SetIpProgHandler(&ipprog);

	TimeSync timeSync;
	if (!ltcParams.IsTimeSyncDisabled()) {
		node.SetTimeSyncHandler(&timeSync);
	}

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_START);
	display.TextStatus(ArtNetConst::MSG_NODE_START, DISPLAY_7SEGMENT_MSG_INFO_NODE_START);

	node.Print();
	node.Start();

	console_status(CONSOLE_GREEN, ArtNetConst::MSG_NODE_STARTED);
	display.TextStatus(ArtNetConst::MSG_NODE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_NODE_STARTED);

	StoreTCNet storetcnet;
	TCNetParams tcnetparams((TCNetParamsStore *) &storetcnet);

	if (tcnetparams.Load()) {
		tcnetparams.Set(&tcnet);
		tcnetparams.Dump();
	}

	DisplayMax7219 max7219(ltcParams.GetMax7219Type(), ltcParams.IsShowSysTime());
	max7219.Init(ltcParams.GetMax7219Intensity());

	const TLtcReaderSource source = ltcParams.GetSource();

	if (source != LTC_READER_SOURCE_MIDI) {
		midi.Init(MIDI_DIRECTION_OUTPUT);
	}

	if ((source != LTC_READER_SOURCE_LTC) && (!tLtcDisabledOutputs.bLtc)) {
		ltcSender.Start();
	}

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
		tcnet.Start(); //FIXME Remove here when MASTER is implemented
		tcnetReader.Start();
		break;
	case LTC_READER_SOURCE_INTERNAL:
		ltcGenerator.Start();
		break;
	default:
		ltcReader.Start();
		break;
	}

	if (bEnableNtp) {

		struct TLtcTimeCode LtcTimeCode;

		LtcTimeCode.nFrames = 0;
		LtcTimeCode.nHours = 0;
		LtcTimeCode.nMinutes = 0;
		LtcTimeCode.nSeconds = 0;
		LtcTimeCode.nType = TC_TYPE_SMPTE;

		ntpServer.SetTimeCode(&LtcTimeCode);
		ntpServer.Print();
		ntpServer.Start();
	}

	midi.Print();
	tcnet.Print();
	ltcGenerator.Print();

	RemoteConfig remoteConfig(REMOTE_CONFIG_LTC, REMOTE_CONFIG_MODE_TIMECODE , 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	display.ClearLine(3);
	display.ClearLine(4);

	display.Printf(3, IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	console_puts("Source : ");
	display.Printf(4, "S: ");

	switch (source) {
	case LTC_READER_SOURCE_ARTNET:
		puts("Art-Net");
		display.PutString("Art-Net");
		break;
	case LTC_READER_SOURCE_MIDI:
		puts("MIDI");
		display.PutString("MIDI");
		break;
	case LTC_READER_SOURCE_TCNET:
		console_puts("TCNet ");
		display.PutString("TCNet ");
		if (tcnet.GetLayer() != TCNET_LAYER_UNDEFINED) {
			console_putc('L');
			display.PutChar('L');
			console_putc(TCNet::GetLayerName(tcnet.GetLayer()));
			display.PutChar(TCNet::GetLayerName(tcnet.GetLayer()));
			console_puts(" Time");
			display.PutString(" Time");

			if (tcnet.IsSetTimeCodeType()) {
				switch (tcnet.GetTimeCodeType()) {
				case TCNET_TIMECODE_TYPE_FILM:
					console_puts(" F24");
					display.PutString(" F24");
					break;
				case TCNET_TIMECODE_TYPE_EBU_25FPS:
					console_puts(" F25");
					display.PutString(" F25");
					break;
				case TCNET_TIMECODE_TYPE_DF:
					console_puts(" F29");
					display.PutString(" F29");
					break;
				case TCNET_TIMECODE_TYPE_SMPTE_30FPS:
					console_puts(" F30");
					display.PutString(" F30");
					break;
				default:
					break;
				}
				puts("FPS");
			}
		} else {
			puts("SMPTE");
			display.PutString("SMPTE");
		}
		break;
	case LTC_READER_SOURCE_INTERNAL:
		puts("Internal");
		display.PutString("Internal");
		break;
	default:
		puts("LTC");
		display.PutString("LTC");
		break;
	}

	if (tLtcDisabledOutputs.bDisplay) {
		printf(" Display is disabled\n");
	}

	if (tLtcDisabledOutputs.bMax7219) {
		printf(" Max7219 is disabled\n");
	}

	if ((source != LTC_READER_SOURCE_LTC) && (tLtcDisabledOutputs.bLtc)) {
		printf(" LTC output is disabled\n");
	}

	if ((source != LTC_READER_SOURCE_TCNET) && (tLtcDisabledOutputs.bTCNet)) {
		printf(" TCNet output is disabled\n");
	}

	if ((source != LTC_READER_SOURCE_MIDI) && (tLtcDisabledOutputs.bMidi)) {
		printf(" MIDI output is disabled\n");
	}

	if ((source != LTC_READER_SOURCE_ARTNET) && (tLtcDisabledOutputs.bArtNet)) {
		printf(" Art-Net output is disabled\n");
	}

	if (tLtcDisabledOutputs.bNtp) {
		printf(" NTP output is disabled\n");
	}

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();

		nw.Run();
		node.Run();
		if (source == LTC_READER_SOURCE_TCNET) {	//FIXME Remove when MASTER is implemented
			tcnet.Run();
		}

		switch (source) {
		case LTC_READER_SOURCE_LTC:
			ltcReader.Run();
			break;
		case LTC_READER_SOURCE_ARTNET:
			artnetReader.Run();	// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_MIDI:
			midiReader.Run();
			break;
		case LTC_READER_SOURCE_TCNET:
			tcnetReader.Run();	// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_INTERNAL:
			ltcGenerator.Run();	// Handles MIDI Quarter Frame output messages
			break;
		default:
			break;
		}

		if (bEnableNtp) {
			ntpServer.Run();
		}

		remoteConfig.Run();
		spiFlashStore.Flash();

		if (tLtcDisabledOutputs.bDisplay) {
			display.Run();
		}
	}
}

}
