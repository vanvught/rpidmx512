/**
 * @file tcnetpackets.h
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

struct TManagementHeader {
	uint16_t NodeID;
	uint8_t ProtocolVersionMajor;
	uint8_t ProtocolVersionMinor;
	uint8_t Header[3];
	uint8_t MessageType;
	uint8_t NodeName[TCNET_NODE_NAME_LENGTH];
	uint8_t SEQ;
	uint8_t NodeType;
	uint16_t NodeOptions;
	uint32_t TimeStamp;
}PACKED;

struct TOptIn {
	struct TManagementHeader ManagementHeader;
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

struct TOptOut {
	struct TManagementHeader ManagementHeader;
	uint16_t NodeCount;
	uint16_t NodeListenerPort;
}PACKED;

struct TStatus {
	struct TManagementHeader ManagementHeader;
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
	uint8_t Reserved1;
	uint8_t SMPTEMode;
	uint8_t AutoMasterMode;
	uint8_t Reserved2;
	uint8_t AppSpecific;
}PACKED;

struct TTimeSync {
	struct TManagementHeader ManagementHeader;
	uint8_t STEP;
	uint8_t Reserved1;
	uint16_t NodeListenerPort;
	uint32_t RemoteTimestamp;
}PACKED;

struct TErrorNotification {
	struct TManagementHeader ManagementHeader;
	uint8_t Datatype;
	uint8_t LayerID;
	uint16_t Code;
	uint16_t MessageType;
}PACKED;

struct TRequest {
	struct TManagementHeader ManagementHeader;
	uint8_t Datatype;
	uint8_t Layer;
}PACKED;

struct TApplication {
	struct TManagementHeader ManagementHeader;
	uint8_t Data;
}PACKED;

#define APPLICATION_DATA_DATA_SIZE(x)	((x) - sizeof(struct TManagementHeader ManagementHeader))

struct TControl {
	struct TManagementHeader ManagementHeader;
	uint8_t ControlPath;
}PACKED;

struct TTextData {
	struct TManagementHeader ManagementHeader;
	uint8_t TextData;
}PACKED;

struct TTime {
	struct TManagementHeader ManagementHeader;
	uint32_t L1Time;
	uint32_t L2Time;
	uint32_t L3Time;
	uint32_t L4Time;
	uint32_t LATime;
	uint32_t LBTime;
	uint32_t LMTime;
	uint32_t LCTime;
	uint32_t L1TotalTime;
	uint32_t L2TotalTime;
	uint32_t L3TotalTime;
	uint32_t L4TotalTime;
	uint32_t LATotalTime;
	uint32_t LBTotalTime;
	uint32_t LMTotalTime;
	uint32_t LCTotalTime;
	uint8_t L1BeatMarker;
	uint8_t L2BeatMarker;
	uint8_t L3BeatMarker;
	uint8_t L4BeatMarker;
	uint8_t LABeatMarker;
	uint8_t LBBeatMarker;
	uint8_t LMBeatMarker;
	uint8_t LCBeatMarker;
	uint8_t L1LayerState;
	uint8_t L2LayerState;
	uint8_t L3LayerState;
	uint8_t L4LayerState;
	uint8_t LALayerState;
	uint8_t LBLayerState;
	uint8_t LMLayerState;
	uint8_t LCLayerState;
	uint8_t L1LayerOnAir;
	uint8_t L2LayerOnAir;
	uint8_t L3LayerOnAir;
	uint8_t L4LayerOnAir;
	uint8_t LALayerOnAir;
	uint8_t LBLayerOnAir;
	uint8_t LMLayerOnAir;
	uint8_t LCLayerOnAir;
	uint8_t Reserved1[32];
	uint8_t TimeCodeHours;
	uint8_t TimeCodeMinutes;
	uint8_t TimeCodeSeconds;
	uint8_t TimeCodeFrames;
	uint8_t TimeCodeType;
	uint8_t Reserved2[5];
}PACKED;

struct TTCNetPacket {
	union {
		struct TManagementHeader ManagementHeader;
		struct TOptIn OptIn;
		struct TOptOut OptOut;
		struct TStatus Status;
		struct TTimeSync Timesync;
		struct TErrorNotification ErrorNotification;
		struct TRequest Request;
		struct TApplication Application;
		struct TControl Control;
		struct TTextData TextData;
		struct TTime Time;
	};
	uint8_t filler[512];
}PACKED;

struct TTCNet {
	uint32_t BytesReceived;
	uint32_t IPAddressFrom;
	struct TTCNetPacket TCNetPacket;
};

#endif /* TCNETPACKETS_H_ */
