/**
 * @file rdm.h
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef RDM_H_
#define RDM_H_

#include <stdint.h>

#define RDM_DATA_BUFFER_SIZE			512

///< 3 Timing
#define RDM_TRANSMIT_BREAK_TIME			176		///< Min 176μs
#define RDM_TRANSMIT_MAB_TIME			12		///< Min 12μs
///< 3.2.2 Responder Packet spacing
#define RDM_RESPONDER_PACKET_SPACING	200		///< Min 176μs, Max 2ms

#define RDM_RESPONDER_DATA_DIRECTION_DELAY	4	///<

///< 5 Device Addressing
#define RDM_UID_SIZE  					6		///< 48-bit

///< 6.2.3 Message Length
#define RDM_MESSAGE_MINIMUM_SIZE		24		///< Minimum size of RDM message without the checksum
#define RDM_MESSAGE_CHECKSUM_SIZE		2		///< Size of the checksum
#define RDM_MESSAGE_COUNT_MAX			255		///< Message Count field for Responder Generated Messages

///< 7.6 Discovery Mute/Un-Mute Messages
///< 7.6.1 Control Field
///< The Reserved bits (Bits 4-15) are reserved for future implementation and shall be set to 0.
#define RDM_CONTROL_FIELD_MANAGED_PROXY_FLAG	1 << 0	///< The Managed Proxy Flag (Bit 0) shall be set to 1 when the responder is a Proxy device.
#define RDM_CONTROL_FIELD_SUB_DEVICE_FLAG		1 << 1	///< The Sub-Device Flag (Bit 1) shall be set to 1 when the responder supports Sub-Devices.
#define RDM_CONTROL_FIELD_BOOTLOADER_FLAG		1 << 2	///< The Boot-Loader Flag (Bit 2) shall only be set to 1 when the device is incapable of normal operation until receiving a firmware upload.
#define RDM_CONTROL_FIELD_PROXIED_DEVICE_FLAG	1 << 3	///< The Proxied Device Flag (Bit 3) shall only be set to 1 when a Proxy is responding to Discovery on behalf of another device. This flag indicates that the response has come from a Proxy, rather than the actual device.

struct _rdm_command
{
	uint8_t start_code;						///< 1			SC_RDM
	uint8_t sub_start_code;					///< 2			SC_SUB_MESSAGE
	uint8_t message_length;					///< 3			Range 24 to 255
	uint8_t destination_uid[RDM_UID_SIZE];	///< 4,5,6,7,8,9
	uint8_t source_uid[RDM_UID_SIZE];		///< 10,11,12,13,14,15
	uint8_t transaction_number;				///< 16
union {
	uint8_t port_id;						///< 17
	uint8_t response_type;					///< 17
} slot16;
	uint8_t message_count;					///< 18
	uint8_t sub_device[2];					///< 19, 20
	uint8_t command_class;					///< 21
	uint8_t param_id[2];					///< 22, 23
	uint8_t param_data_length;				///< 24			PDL	Range 0 to 231
	uint8_t param_data[231];				///< 25,,,,		PD	6.2.3 Message Length
} ;

struct _rdm_discovery_msg {
	uint8_t header_FE[7];
	uint8_t header_AA;
	uint8_t masked_device_id[12];
	uint8_t checksum[4];
} ;

///< http://rdm.openlighting.org/pid/display?manufacturer=0&pid=96
struct _rdm_device_info
{
	uint8_t protocol_major;			///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t protocol_minor;			///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t device_model[2];		///< This field identifies the Device Model ID of the Root Device or the Sub-Device. The Manufacturer shall not use the same ID to represent more than one unique model type.
	uint8_t product_category[2];	///< Devices shall report a Product Category based on the product’s primary function.
	uint8_t software_version[4];	///< This field indicates the Software Version ID for the device. The Software Version ID is a 32-bit value determined by the Manufacturer.
	uint8_t dmx_footprint[2];		///< If the DEVICE_INFO message is directed to a Sub-Device, then the response for this field contains the DMX512 Footprint for that Sub-Device. If the message is sent to the Root Device, it is the Footprint for the Root Device itself. If the Device or Sub-Device does not utilize Null Start Code packets for any control or functionality then it shall report a Footprint of 0x0000.
	uint8_t current_personality;	///<
	uint8_t personality_count;		///<
	uint8_t dmx_start_address[2];	///< If the Device or Sub-Device that this message is directed to has a DMX512 Footprint of 0, then this field shall be set to 0xFFFF.
	uint8_t sub_device_count[2];	///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t sensor_count;			///< This field indicates the number of available sensors in a Root Device or Sub-Device. When this parameter is directed to a Sub-Device, the reply shall be identical for any Sub-Device owned by a specific Root Device.
};

///< Personalities
struct _rdm_personality
{
	uint16_t slots;
	const char *description;
};

// Unique identifier (UID) which consists of a 2 byte ESTA manufacturer ID, and a 4 byte device ID.
static const uint8_t UID_ALL[RDM_UID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

extern void rdm_send_data(const uint8_t *, const uint16_t);

#endif /* RDM_H_ */
