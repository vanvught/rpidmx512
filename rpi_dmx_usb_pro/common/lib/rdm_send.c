/**
 * @file rdm_send.c
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

#include <stdint.h>
#include <string.h>

#include "bcm2835.h"
#include "bcm2835_pl011.h"
#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "rdm_device_info.h"

static uint8_t rdm_message_count;

/**
 * @ingroup rdm
 *
 *
 * @param data
 * @param data_length
 */
void rdm_send_data(const uint8_t *data, const uint16_t data_length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(RDM_TRANSMIT_BREAK_TIME);	// Break Time

	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(RDM_TRANSMIT_MAB_TIME);	// Mark After Break

	uint16_t i = 0;
	for (i = 0; i < data_length; i++)
	{
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = data[i];
	}
	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
	udelay(44);
}

/**
 *
 * @param data
 * @param data_length
 */
static void rdm_send_no_break(const uint8_t *data, const uint16_t data_length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;

	uint16_t i = 0;
	for (i = 0; i < data_length; i++)
	{
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = data[i];
	}
	while (1)
	{
		if ((BCM2835_PL011->FR & 0x20) == 0)
			break;
	}
}

/**
 *
 * @param data
 * @param data_length
 */
void rdm_send_discovery_respond_message(const uint8_t *data, const uint16_t data_length)
{
	const uint64_t delay = bcm2835_st_read() - rdm_data_receive_end_get();
	// 3.2.2 Responder Packet spacing
	if (delay < RDM_RESPONDER_PACKET_SPACING)
	{
		udelay(RDM_RESPONDER_PACKET_SPACING - delay);
	}

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);
	rdm_send_no_break(data, data_length);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

/**
 *
 * @param is_ack
 * @param reason
 */
static void rdm_send_respond_message(uint8_t response_type, uint16_t value)
{
	uint16_t rdm_checksum = 0;

	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;

	switch (response_type) {
		case E120_RESPONSE_TYPE_ACK:
			rdm_response->slot16.response_type = E120_RESPONSE_TYPE_ACK;
			break;
		case E120_RESPONSE_TYPE_NACK_REASON:
		case E120_RESPONSE_TYPE_ACK_TIMER:
			rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
			rdm_response->slot16.response_type = response_type;
			rdm_response->param_data_length = 2;
			rdm_response->param_data[0] = (uint8_t)(value >> 8);
			rdm_response->param_data[1] = (uint8_t)value;
			break;
		default:
			// forces timeout
			return;
			break;
	}

	const uint8_t *uid_device = rdm_device_info_get_uuid();

	memcpy(rdm_response->destination_uid, rdm_response->source_uid, RDM_UID_SIZE);
	memcpy(rdm_response->source_uid, uid_device, RDM_UID_SIZE);

	rdm_response->command_class++;

	uint8_t i = 0;
	for (i = 0; i < rdm_response->message_length; i++)
	{
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = rdm_checksum >> 8;
	rdm_data[i] = rdm_checksum & 0XFF;;

	const uint64_t delay = bcm2835_st_read() - rdm_data_receive_end_get();
	// 3.2.2 Responder Packet spacing
	if (delay < RDM_RESPONDER_PACKET_SPACING)
	{
		udelay(RDM_RESPONDER_PACKET_SPACING - delay);
	}

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);
	rdm_send_data(rdm_data, rdm_response->message_length + RDM_MESSAGE_CHECKSUM_SIZE);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

/**
 *
 */
void rdm_send_respond_message_ack()
{
	rdm_send_respond_message(E120_RESPONSE_TYPE_ACK, 0);
}

/**
 *
 * @param reason
 */
void rdm_send_respond_message_nack(const uint16_t reason)
{
	rdm_send_respond_message(E120_RESPONSE_TYPE_NACK_REASON, reason);
}

/**
 *
 * @param timer
 */
void rdm_send_respond_message_ack_timer(const uint16_t timer)
{
	rdm_send_respond_message(E120_RESPONSE_TYPE_ACK_TIMER, timer);
}

/**
* Increment the queued message count
*/
void rdm_send_increment_message_count()
{
	if (rdm_message_count != RDM_MESSAGE_COUNT_MAX)
		rdm_message_count++;
}

/**
 * Decrement the queued message count
 */
void rdm_send_decrement_message_count()
{
	if (rdm_message_count)
		rdm_message_count--;
}

