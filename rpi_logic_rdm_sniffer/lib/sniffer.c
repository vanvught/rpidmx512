/**
 * @file sniffer.c
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
#include <stdio.h>
#include <stdbool.h>

#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "monitor.h"
#include "sniffer.h"

static volatile struct _rdm_statistics rdm_statistics __attribute__((aligned(4)));

/**
 * @ingroup sniffer
 *
 * @return
 */
const volatile struct _rdm_statistics *rdm_statistics_get(void) {
	return &rdm_statistics;
}

/**
 * @ingroup sniffer
 *
 * This function is called from the poll table in \ref main.c
 */
void sniffer_dmx(void) {
	if (!dmx_get_available())
		return;

	dmx_set_available_false();

	monitor_dmx_data(MONITOR_LINE_DMX_DATA);
}

/**
 * @ingroup sniffer
 *
 * This function is called from the poll table in \ref main.c
 */
void sniffer_rdm(void) {
	const uint8_t *rdm_data = rdm_get_available();

	if (rdm_data == NULL)
		return;

	if (rdm_data[0] == E120_SC_RDM) {
		struct _rdm_command *p = (struct _rdm_command *) (rdm_data);
		switch (p->command_class) {
		case E120_DISCOVERY_COMMAND:
			rdm_statistics.discovery_packets++;
			break;
		case E120_DISCOVERY_COMMAND_RESPONSE:
			rdm_statistics.discovery_response_packets++;
			break;
		case E120_GET_COMMAND:
			rdm_statistics.get_requests++;
			break;
		case E120_SET_COMMAND:
			rdm_statistics.set_requests++;
			break;
		default:
			break;
		}
	} else if (rdm_data[0] == 0xFE) {
		rdm_statistics.discovery_response_packets++;
	}
}
