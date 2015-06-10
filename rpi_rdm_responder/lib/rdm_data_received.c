/**
 * @file rdm_data_received.c
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

#include <stddef.h>

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "rdm_handle_data.h"

/**
 * @ingroup rdm
 *
 * The function is registered in the poll table \file main.c
 */
void rdm_data_received(void)
{
	uint8_t *rdm_data = (uint8_t *)rdm_get_available();

	if (rdm_data == NULL)
		return;

	const struct _rdm_command *rdm_cmd = (struct _rdm_command *)rdm_data;

	const uint8_t command_class = rdm_cmd->command_class;

	switch (command_class) {
		case E120_DISCOVERY_COMMAND:
		case E120_GET_COMMAND:
		case E120_SET_COMMAND:
			rdm_handle_data(rdm_data);
			break;
		default:
			break;
	}
}
