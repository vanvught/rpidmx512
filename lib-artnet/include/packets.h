/**
 * @file packets.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef PACKETS_H_
#define PACKETS_H_

#include <stdint.h>

#include "common.h"

#if  ! defined (PACKED)
#define PACKED __attribute__((packed))
#endif

/**
 * Table 1 - OpCodes
 * The supported legal OpCode values used in Art-Net packets
 */
enum TOpCodes {
	OP_POLL = 0x2000,		///< This is an ArtPoll packet, no other data is contained in this UDP packet.
	OP_POLLREPLY = 0x2100,	///< This is an ArtPollReply Packet. It contains device status information.
	OP_DIAGDATA = 0x2300,	///< Diagnostics and data logging packet.
	OP_DMX = 0x5000,		///< This is an ArtDmx data packet. It contains zero start code DMX512 information for a single Universe.
	OP_SYNC = 0x5200,		///< This is an ArtSync data packet. It is used to force synchronous transfer of ArtDmx packets to a node’s output.
	OP_ADDRESS = 0x6000,	///< This is an ArtAddress packet. It contains remote programming information for a Node.
	OP_NOT_DEFINED = 0x0000	///< OP_NOT_DEFINED
};

/**
 * ArtPoll packet definition
 */
struct TArtPoll {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the OpCode listing.
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t TalkToMe;		///< Set behavior of Node
	uint8_t Priority;		///< The lowest priority of diagnostics message that should be sent. See \ref
}PACKED;

/**
 * ArtPollReply packet definition
 */
struct TArtPollReply {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpPollReply \ref TOpCodes
	uint8_t IPAddress[4];	///< Array containing the Node’s IP address. First array entry is most significant byte of address.
	uint16_t Port;			///< The Port is always 0x1936
	uint8_t VersInfoH;		///< High byte of Node’s firmware revision number.
	uint8_t VersInfoL;		///< Low byte of Node’s firmware revision number.
	uint8_t NetSwitch;		///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
	uint8_t SubSwitch;		///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
	uint8_t OemHi;			///< The high byte of the Oem value
	uint8_t Oem;			///< The low byte of the Oem value. The Oem word describes the equipment vendor and the feature set available.
	uint8_t Ubea;			///< This field contains the firmware version of the User Bios Extension Area (UBEA). If the UBEA is not programmed, this field contains zero.
	uint8_t Status1;		///< General Status register
	uint8_t EstaMan[2];		///< The ESTA manufacturer code. These codes are used to represent equipment manufacturer. They are assigned by ESTA.
	uint8_t ShortName[ARTNET_SHORT_NAME_LENGTH];///< The array represents a null terminated short name for the Node.
	uint8_t LongName[ARTNET_LONG_NAME_LENGTH];	///< The array represents a null terminated long name for the Node.
	uint8_t NodeReport[ARTNET_REPORT_LENGTH];	///< The array is a textual report of the Node’s operating status or operational errors. It is primarily intended for ‘engineering’ data rather than ‘end user’ data.
	uint8_t NumPortsHi;		///< The high byte of the word describing the number of input or output ports. The high byte is for future expansion and is currently zero.
	uint8_t NumPortsLo;		///< The low byte of the word describing the number of input or output ports.
	uint8_t PortTypes[ARTNET_MAX_PORTS];	///< This array defines the operation and protocol of each channel.
	uint8_t GoodInput[ARTNET_MAX_PORTS];	///< This array defines input status of the node.
	uint8_t GoodOutput[ARTNET_MAX_PORTS];	///< This array defines output status of the node.
	uint8_t SwIn[ARTNET_MAX_PORTS];			///< Bits 3-0 of the 15 bit Port-Address for each of the 4 possible input ports are encoded into the low nibble.
	uint8_t SwOut[ARTNET_MAX_PORTS];		///< Bits 3-0 of the 15 bit Port-Address for each of the 4 possible output ports are encoded into the low nibble.
	uint8_t SwVideo;		///< The field is now deprecated
	uint8_t SwMacro;		///< If the Node supports macro key inputs, this byte represents the trigger values.
	uint8_t SwRemote;		///< If the Node supports remote trigger inputs, this byte represents the trigger values.
	uint8_t Spare1;			///< Not used, set to zero
	uint8_t Spare2;			///< Not used, set to zero
	uint8_t Spare3;			///< Not used, set to zero
	uint8_t Style;			///< The Style code defines the equipment style of the device. See \ref TNodeStyleCode
	uint8_t MAC[ARTNET_MAC_SIZE];	///< MAC Address
	uint8_t BindIp[4];		///< If this unit is part of a larger or modular product, this is the IP of the root device.
	uint8_t BindIndex;		///< Set to zero if no binding, otherwise this number represents the order of bound devices. A lower number means closer to root device. A value of 1 means root device.
	uint8_t Status2;		///<
	uint8_t Filler[26];		///< Transmit as zero. For future expansion.
}PACKED;

/**
 * ArtDmx is the data packet used to transfer DMX512 data.
 */
struct TArtDmx {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpDmx \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Sequence;		///< The sequence number is used to ensure that ArtDmx packets are used in the correct order.
	uint8_t Physical;		///< The physical input port from which DMX512 data was input. This field is for information only. Use Universe for data routing.
	uint16_t PortAddress;	///< The 15 bit Port-Address to which this packet is destined.
	uint8_t LengthHi;		///< The length of the DMX512 data array. This value should be an even number in the range 2 – 512.
	uint8_t Length;			///< Low Byte of above.
	uint8_t Data[ARTNET_DMX_LENGTH];///< A variable length array of DMX512 lighting data.
}PACKED;

/**
 * ArtDiagData is a general purpose packet that allows a node or controller to send diagnostics data for display.
 */
struct TArtDiagData {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpDiagData See \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Ignore by receiver, set to zero by sender
	uint8_t Priority;		///< The priority of this diagnostic data. See \ref TPriorityCodes
	uint8_t Filler2;		///< Ignore by receiver, set to zero by sender
	uint8_t Filler3;		///< Ignore by receiver, set to zero by sender
	uint8_t LengthHi;		///< The length of the text array below. High Byte.
	uint8_t LengthLo;		///< Low byte
	uint8_t Data[512];		///< ASCII text array, null terminated. Max length is 512 bytes including the null terminator. // TODO #define
}PACKED;

/**
 * ArtSync packet definition
 */
struct TArtSync {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpSync \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Aux1;			///< Transmit as zero.
	uint8_t Aux2;			///< Transmit as zero.
}PACKED;

/**
 * ArtAddress packet definition
 *
 * Fields 5 to 13 contain the data that will be programmed into the node.
 */
struct TArtAddress {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t NetSwitch;		///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t Filler2;		///< Pad length to match ArtPoll.
	uint8_t ShortName[ARTNET_SHORT_NAME_LENGTH];///< The Node will ignore this value if the string is null.
	uint8_t LongName[ARTNET_LONG_NAME_LENGTH];	///< The Node will ignore this value if the string is null.
	uint8_t SwIn[ARTNET_MAX_PORTS];		///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t SwOut[ARTNET_MAX_PORTS];	///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t SubSwitch;		///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t SwVideo;		///< Reserved
	uint8_t Command;		///< Node configuration commands \ref TArtnetPortCommand
}PACKED;

/**
 * union of supported artnet packets
 */
union UArtPacket{
  struct TArtPoll ArtPoll;			///< ArtPoll packet
  struct TArtPollReply ArtPollReply;///< ArtPollReply packet
  struct TArtDmx ArtDmx;			///< ArtDmx packet
  struct TArtDiagData ArtDiagData;	///< ArtDiagData packet
  struct TArtSync ArtSync;			///< ArtSync packet
  struct TArtAddress ArtAddress;	///< ArtAddress packet
};


/**
 * a packet, containing data, length, type and a src/dst address
 */
struct TArtNetPacket {
	int length;						///<
	uint32_t IPAddressFrom;			///<
	uint32_t IPAddressTo;			///<
	TOpCodes OpCode;				///<
	union UArtPacket ArtPacket;		///<
};

#endif /* PACKETS_H_ */
