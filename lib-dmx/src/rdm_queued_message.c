/**
 * @file rdm_queued_message.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>

#include "rdm_queued_message.h"

#include "rdm.h"
#include "rdm_e120.h"
#include "rdm_send.h"

#include "util.h"

static uint8_t message_count = (uint8_t) 0;
static bool never_queued = true;

static struct _rdm_queued_message queue[255] ALIGNED;

static void _copy(struct _rdm_command *rdm, const uint8_t index) {
	uint8_t i;

	rdm->command_class = queue[index].command_class;
	rdm->param_id[0] = queue[index].param_id[0];
	rdm->param_id[1] = queue[index].param_id[1];
	rdm->param_data_length = queue[index].param_data_length;

	for (i = 0; i < rdm->param_data_length; i++) {
		rdm->param_data[i] = queue[index].param_data[i];
	}

}

const uint8_t rdm_queued_message_get_count(void) {
	return message_count;
}

void rdm_queued_message_handler(uint8_t *rdm_data) {
	struct _rdm_command *rdm_response = (struct _rdm_command *) rdm_data;

	if (never_queued) {
		rdm_response->slot16.response_type = E120_STATUS_MESSAGES;
		rdm_response->param_data_length = 0;
	} else if (rdm_response->param_data[0] == E120_STATUS_GET_LAST_MESSAGE) {
		_copy(rdm_response, message_count);
	} else {
		if (message_count != 0) {
			message_count--;
			_copy(rdm_response, message_count);
		} else {
			rdm_response->slot16.response_type = E120_STATUS_MESSAGES;
			rdm_response->param_data_length = 0;
		}
	}

	rdm_send_respond_message_ack(rdm_data);
}

bool rdm_queued_message_add(const struct _rdm_queued_message *msg) {
	uint8_t i;

	never_queued = false;

	if (message_count != RDM_MESSAGE_COUNT_MAX) {

		queue[message_count].command_class = msg->command_class;
		queue[message_count].param_id[0] = msg->param_id[0];
		queue[message_count].param_id[1] = msg->param_id[1];
		queue[message_count].param_data_length = msg->param_data_length;

		for (i = 0; i < msg->param_data_length; i++) {
			queue[message_count].param_data[i] = msg->param_data[i];
		}

		message_count++;

		return true;
	}

	return false;
}
