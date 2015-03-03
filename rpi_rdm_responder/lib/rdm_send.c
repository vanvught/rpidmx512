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
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

// TODO move for util.h
typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;

extern uint8_t rdm_data[60];

void rdm_send_discovery_msg(const uint8_t *data, const uint16_t data_length)
{
	uint64_t delay = bcm2835_st_read() - rdm_data_receive_end_get();

	if (delay < 180)
	{
		udelay(180 - delay);
	}

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);

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

	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}

void rdm_send_respond_message(const uint8_t UID_DEVICE[UID_SIZE], uint8_t rdm_packet_is_handled)
{
	uint16_t rdm_checksum = 0;

	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;

	if(rdm_packet_is_handled == TRUE)
	{
		rdm_response->port_id = E120_RESPONSE_TYPE_ACK;
	} else
	{
		rdm_response->message_length = rdm_response->message_length + 2;
		rdm_response->port_id = E120_RESPONSE_TYPE_NACK_REASON;
		rdm_response->param_data_length = 2;
		rdm_response->param_data[0] = 0;
		rdm_response->param_data[1] = 0;
	}

	memcpy(rdm_response->destination_uid, rdm_response->source_uid, UID_SIZE);
	memcpy(rdm_response->source_uid, UID_DEVICE, UID_SIZE);

	rdm_response->command_class++;

	uint8_t i = 0;
	for (i = 0; i < rdm_response->message_length; i++)
	{
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = rdm_checksum >> 8;
	rdm_data[i] = rdm_checksum & 0XFF;;

	uint64_t delay = bcm2835_st_read() - rdm_data_receive_end_get();

	if (delay < 180)
	{
		udelay(180 - delay);
	}

	dmx_port_direction_set(DMX_PORT_DIRECTION_OUTP, FALSE);
	dmx_data_send(rdm_data, rdm_response->message_length + 2);
	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);
}
