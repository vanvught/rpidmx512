/**
 * @file tcnetpackets.h
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

/**
 * Specification V3.3.3 11/11/2019
 */

#ifndef TCNETPACKETS_H_
#define TCNETPACKETS_H_

#include <stdint.h>

enum TTCNetMessageType {
	TCNET_MESSAGE_TYPE_OPTIN = 2,
	TCNET_MESSAGE_TYPE_OPTOUT = 3,
	TCNET_MESSAGE_TYPE_STATUS = 5,
	TCNET_MESSAGE_TYPE_TIMESYNC = 10,
	TCNET_MESSAGE_TYPE_ERROR_NOTIFICTION = 13,
	TCNET_MESSAGE_TYPE_REQUEST = 20,
	TCNET_MESSAGE_TYPE_APPLICATION = 30,
	TCNET_MESSAGE_TYPE_CONTROL = 101,
	TCNET_MESSAGE_TYPE_TEXTDATA = 128,
	TCNET_MESSAGE_TYPE_TIME = 254
};

enum TTCNetNodeType {
	TCNET_TYPE_AUTO = 1,
	TCNET_TYPE_MASTER = 2,
	TCNET_TYPE_SLAVE = 4,
	TCNET_TYPE_REPEATER = 8
};

enum TTCNetNodeName {
	TCNET_NODE_NAME_LENGTH = 8
};

enum TTCNetVenderName {
	TCNET_VENDOR_NAME_LENGTH = 16
};

enum TTCNetDeviceName {
	TCNET_DEVICE_NAME_LENGTH = 16
};

#if  !defined (PACKED)
#define PACKED __attribute__((packed))
#endif

struct TTCNetPacketManagementHeader {
	uint16_t NodeID;							//  0:2
	uint8_t ProtocolVersionMajor;				//  2:1
	uint8_t ProtocolVersionMinor;				//  3:1
	uint8_t Header[3];							//  4:3
	uint8_t MessageType;						//  7:1
	uint8_t NodeName[TCNET_NODE_NAME_LENGTH];	//  8:8
	uint8_t SEQ;								// 16:1
	uint8_t NodeType;							// 17:1
	uint16_t NodeOptions;						// 18:2
	uint32_t TimeStamp;							// 20:4
}PACKED;

struct TTCNetPacketOptIn {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint16_t NodeCount;
	uint16_t NodeListenerPort;
	uint16_t Uptime;
	uint8_t Reserved1[2];
	uint8_t VendorName[TCNET_VENDOR_NAME_LENGTH];
	uint8_t DeviceName[TCNET_DEVICE_NAME_LENGTH];
	uint8_t DeviceMajorVersion;
	uint8_t DeviceMinorVersion;
	uint8_t DeviceBugVersion;
	uint8_t Reserved2;
}PACKED;

struct TTCNetPacketOptOut {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint16_t NodeCount;
	uint16_t NodeListenerPort;
}PACKED;

struct TTCNetPacketStatus {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t NodeCount[2];			// 24:2
	uint8_t NodeListenerPort[2];	// 26:2
	uint8_t Reserved1[6];			// 28:6
	uint8_t Layer1Source;
	uint8_t Layer2Source;
	uint8_t Layer3Source;
	uint8_t Layer4Source;
	uint8_t LayerASource;
	uint8_t LayerBSource;
	uint8_t LayerMSource;
	uint8_t LayerCSource;
	uint8_t Layer1Status;
	uint8_t Layer2Status;
	uint8_t Layer3Status;
	uint8_t Layer4Status;
	uint8_t LayerAStatus;
	uint8_t LayerBStatus;
	uint8_t LayerMStatus;
	uint8_t LayerCStatus;
	uint32_t Layer1TrackID;
	uint32_t Layer2TrackID;
	uint32_t Layer3TrackID;
	uint32_t Layer4TrackID;
	uint32_t LayerATrackID;
	uint32_t LayerBTrackID;
	uint32_t LayerMTrackID;
	uint32_t LayerCTrackID;
	uint8_t Reserved2;
	uint8_t SMPTEMode;
	uint8_t AutoMasterMode;
	uint8_t Reserved3[15];
	uint8_t AppSpecific[72];
}PACKED;

struct TTCNetPacketTimeSync {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t STEP;
	uint8_t Reserved1;
	uint16_t NodeListenerPort;
	uint32_t RemoteTimestamp;
}PACKED;

struct TTCNetPacketErrorNotification {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t Datatype;
	uint8_t LayerID;
	uint16_t Code;
	uint16_t MessageType;
}PACKED;

struct TTCNetPacketRequest {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t Datatype;
	uint8_t Layer;
}PACKED;

struct TTCNetPacketApplication {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t Data;
}PACKED;

#define APPLICATION_DATA_DATA_SIZE(x)	((x) - sizeof(struct TTCNetPacketManagementHeader ManagementHeader))

struct TTCNetPacketControl {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t ControlPath;
}PACKED;

struct TTCNetPacketTextData {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint8_t TextData;
}PACKED;

struct TTCNetPacketTimeTimeCode {
	uint8_t SMPTEMode;
	uint8_t State;
	uint8_t Hours;
	uint8_t Minutes;
	uint8_t Seconds;
	uint8_t Frames;
} PACKED;

struct TTCNetPacketTime {
	struct TTCNetPacketManagementHeader ManagementHeader;
	uint32_t L1Time;							//  24:4
	uint32_t L2Time;							//  28:4
	uint32_t L3Time;							//  32:4
	uint32_t L4Time;							//  36:4
	uint32_t LATime;							//  40:4
	uint32_t LBTime;							//  44:4
	uint32_t LMTime;							//  48:4
	uint32_t LCTime;							//  52:4
	uint32_t L1TotalTime;						//  56:4
	uint32_t L2TotalTime;						//  60:4
	uint32_t L3TotalTime;						//  64:4
	uint32_t L4TotalTime;						//  68:4
	uint32_t LATotalTime;						//  72:4
	uint32_t LBTotalTime;						//  76:4
	uint32_t LMTotalTime;						//  80:4
	uint32_t LCTotalTime;						//  84:4
	uint8_t L1BeatMarker;						//  88:1
	uint8_t L2BeatMarker;						//  89:1
	uint8_t L3BeatMarker;						//  90:1
	uint8_t L4BeatMarker;						//  91:1
	uint8_t LABeatMarker;						//  92:1
	uint8_t LBBeatMarker;						//  93:1
	uint8_t LMBeatMarker;						//  94:1
	uint8_t LCBeatMarker;						//  95:1
	uint8_t L1LayerState;						//  96:1
	uint8_t L2LayerState;						//  97:1
	uint8_t L3LayerState;						//  98:1
	uint8_t L4LayerState;						//  99:1
	uint8_t LALayerState;						// 100:1
	uint8_t LBLayerState;						// 101:1
	uint8_t LMLayerState;						// 102:1
	uint8_t LCLayerState;						// 103:1
	uint8_t Reserved1;							// 104:1
	uint8_t SMPTEMode;							// 105:1
	struct TTCNetPacketTimeTimeCode L1TimeCode;	// 106:6
	struct TTCNetPacketTimeTimeCode L2TimeCode;	// 112:6
	struct TTCNetPacketTimeTimeCode L3TimeCode;	// 118:6
	struct TTCNetPacketTimeTimeCode L4TimeCode;	// 124:6
	struct TTCNetPacketTimeTimeCode LATimeCode;	// 130:6
	struct TTCNetPacketTimeTimeCode LBTimeCode;	// 136:6
	struct TTCNetPacketTimeTimeCode LMTimeCode;	// 142:6
	struct TTCNetPacketTimeTimeCode LCTimeCode;	// 148:6
	uint8_t L1LayerOnAir;						// 154:1
	uint8_t L2LayerOnAir;						// 155:1
	uint8_t L3LayerOnAir;						// 156:1
	uint8_t L4LayerOnAir;						// 157:1
	uint8_t LALayerOnAir;						// 158:1
	uint8_t LBLayerOnAir;						// 159:1
	uint8_t LMLayerOnAir;						// 160:1
	uint8_t LCLayerOnAir;						// 161:1
	uint8_t Reserved2;							// 162:1
} PACKED;

struct TTCNetPacket {
	union {
		struct TTCNetPacketManagementHeader ManagementHeader;
		struct TTCNetPacketOptIn OptIn;
		struct TTCNetPacketOptOut OptOut;
		struct TTCNetPacketStatus Status;
		struct TTCNetPacketTimeSync Timesync;
		struct TTCNetPacketErrorNotification ErrorNotification;
		struct TTCNetPacketRequest Request;
		struct TTCNetPacketApplication Application;
		struct TTCNetPacketControl Control;
		struct TTCNetPacketTextData TextData;
		struct TTCNetPacketTime Time;
	};
	uint8_t filler[512];
} PACKED;

struct TTCNet {
	uint32_t BytesReceived;
	uint32_t IPAddressFrom;
	struct TTCNetPacket TCNetPacket;
};

#endif /* TCNETPACKETS_H_ */
