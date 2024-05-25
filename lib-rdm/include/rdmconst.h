/**
 * @file rdmconst.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMCONST_H_
#define RDMCONST_H_

#include <cstdint>

#define RDM_DATA_BUFFER_SIZE					512									///<
#define RDM_DATA_BUFFER_INDEX_ENTRIES			(1 << 4)							///<
#define RDM_DATA_BUFFER_INDEX_MASK 				(RDM_DATA_BUFFER_INDEX_ENTRIES - 1)	///<

#define RDM_ROOT_DEVICE							0		///<

///< 3 Timing
#define RDM_TRANSMIT_BREAK_TIME					176		///< Min 176us
#define RDM_TRANSMIT_MAB_TIME					12		///< Min 12us

///< 3.2.2 Responder Packet spacing
#define RDM_RESPONDER_PACKET_SPACING			200		///< Min 176us, Max 2ms

#define RDM_RESPONDER_DATA_DIRECTION_DELAY		4		///<

///< 5 Device Addressing
#define RDM_UID_SIZE  							6		///< 48-bit

///< 6.2.3 Message Length
#define RDM_MESSAGE_MINIMUM_SIZE				24U		///< Minimum size of RDM message without the checksum
#define RDM_MESSAGE_CHECKSUM_SIZE				2U		///< Size of the checksum
#define RDM_MESSAGE_COUNT_MAX					255U	///< Message Count field for Responder Generated Messages

///< 7.6 Discovery Mute/Un-Mute Messages
///< 7.6.1 Control Field
///< The Reserved bits (Bits 4-15) are reserved for future implementation and shall be set to 0.
#define RDM_CONTROL_FIELD_MANAGED_PROXY_FLAG	(1 << 0)	///< The Managed Proxy Flag (Bit 0) shall be set to 1 when the responder is a Proxy device.
#define RDM_CONTROL_FIELD_SUB_DEVICE_FLAG		(1 << 1)	///< The Sub-Device Flag (Bit 1) shall be set to 1 when the responder supports Sub-Devices.
#define RDM_CONTROL_FIELD_BOOTLOADER_FLAG		(1 << 2)	///< The Boot-Loader Flag (Bit 2) shall only be set to 1 when the device is incapable of normal operation until receiving a firmware upload.
#define RDM_CONTROL_FIELD_PROXIED_DEVICE_FLAG	(1 << 3)	///< The Proxied Device Flag (Bit 3) shall only be set to 1 when a Proxy is responding to Discovery on behalf of another device. This flag indicates that the response has come from a Proxy, rather than the actual device.

#define RDM_DEVICE_MANUFACTURER_ID_LENGTH		2	///<

///< 10.5.4 Get Manufacturer Label (MANUFACTURER_LABEL)
#define RDM_MANUFACTURER_LABEL_MAX_LENGTH		32			///< Manufacturer name for the device of up to 32 characters.

///< 10.5.5 Get/Set Device Label (DEVICE_LABEL)
#define RDM_DEVICE_LABEL_MAX_LENGTH				32

///< 10.5.8 Get/Set Language (LANGUAGE)
#define RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH	2			///< The Language Codes are 2 character alpha codes as defined by ISO 639-1.

///< 10.5.9 Get Software Version Label (SOFTWARE_VERSION_LABEL)
#define RDM_SOFTWARE_VERSION_LABEL_MAX_LENGTH	32

///< 10.5.10 Get Boot Software Version ID (BOOT_SOFTWARE_VERSION_ID)
#define RDM_BOOT_SOFTWARE_VERSION_ID_LENGTH		4			///< The Boot Software Version ID is a 32-bit value determined by the Manufacturer.

///< 10.5.11 Get Boot Software Version Label (BOOT_SOFTWARE_VERSION_LABEL)
#define RDM_BOOT_SOFTWARE_VERSION_LABEL_MAX_LENGTH	32

///< 10.7.1 Get Sensor Definition (SENSOR_DEFINITION)
#define RDM_SENSORS_ALL							0xFF		///< The sensor number 0xFF is used to address all sensors.

///< 10.8.1 Get/Set Device Hours (DEVICE_HOURS)
#define RDM_DEVICE_HOURS_SIZE					4

///< 10.11.1 Get/Set Identify Device (IDENTIFY_DEVICE)
#define	RDM_IDENTIFY_STATE_OFF				0
#define	RDM_IDENTIFY_STATE_ON				1

#if  ! defined (PACKED)
# define PACKED __attribute__((packed))
#endif

struct TRdmMessage {
	uint8_t start_code;						///< 1	SC_RDM
	uint8_t sub_start_code;					///< 2	SC_SUB_MESSAGE
	uint8_t message_length;					///< 3	Range 24 to 255
	uint8_t destination_uid[RDM_UID_SIZE];	///< 4,5,6,7,8,9
	uint8_t source_uid[RDM_UID_SIZE];		///< 10,11,12,13,14,15
	uint8_t transaction_number;				///< 16
	union {
		uint8_t port_id;					///< 17
		uint8_t response_type;				///< 17
	} slot16;
	uint8_t message_count;					///< 18
	uint8_t sub_device[2];					///< 19, 20
	uint8_t command_class;					///< 21
	uint8_t param_id[2];					///< 22, 23
	uint8_t param_data_length;				///< 24	PDL	Range 0 to 231
	uint8_t param_data[231];				///< 25,,,,	PD	6.2.3 Message Length
	uint8_t checksum_filler[2];
}PACKED;

struct TRdmMessageNoSc {
	uint8_t sub_start_code;					///< 2	SC_SUB_MESSAGE
	uint8_t message_length;					///< 3	Range 24 to 255
	uint8_t destination_uid[RDM_UID_SIZE];	///< 4,5,6,7,8,9
	uint8_t source_uid[RDM_UID_SIZE];		///< 10,11,12,13,14,15
	uint8_t transaction_number;				///< 16
	union {
		uint8_t port_id;					///< 17
		uint8_t response_type;				///< 17
	} slot16;
	uint8_t message_count;					///< 18
	uint8_t sub_device[2];					///< 19, 20
	uint8_t command_class;					///< 21
	uint8_t param_id[2];					///< 22, 23
	uint8_t param_data_length;				///< 24	PDL	Range 0 to 231
	uint8_t param_data[231];				///< 25,,,,	PD	6.2.3 Message Length
	uint8_t checksum_filler[2];
}PACKED;

struct TRdmDiscoveryMsg {
	uint8_t header_FE[7];					///<
	uint8_t header_AA;						///<
	uint8_t masked_device_id[12];			///<
	uint8_t checksum[4];					///<
}PACKED;

// Unique identifier (UID) which consists of a 2 byte ESTA manufacturer ID, and a 4 byte device ID.
static constexpr uint8_t UID_ALL[RDM_UID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

struct RDMConst {
	static const char MANUFACTURER_NAME[];
	static const uint8_t MANUFACTURER_ID[];
};

#endif /* RDMCONST_H_ */
