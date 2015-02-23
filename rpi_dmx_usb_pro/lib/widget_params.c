/**
 * @file widget_params.c
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

#include "widget_params.h"
#include "bcm2835_vc.h"

static struct _widget_params DMXUSBPRO_params = { 4, FIRMWARE_RDM, 9, 1, 40 };
static struct _widget_sn DMXUSBPRO_SN = { DEC2BCD(78), DEC2BCD(56), DEC2BCD(34), DEC2BCD(12) };

void widget_params_init(void)
{
	uint8_t mac_address[6];
	if (bcm2835_vc_get_board_mac_address(mac_address) == 0){
		DMXUSBPRO_SN.bcd_3 = mac_address[2];
		DMXUSBPRO_SN.bcd_2 = mac_address[3];
		DMXUSBPRO_SN.bcd_1 = mac_address[4];
		DMXUSBPRO_SN.bcd_0 = mac_address[5];
	}
}

void widget_params_get(struct _widget_params *widget_params)
{
	memcpy(widget_params, &DMXUSBPRO_params, sizeof(struct _widget_params));
}


void widget_params_break_time_set(uint8_t break_time)
{
	DMXUSBPRO_params.break_time = break_time;
}

void widget_params_mab_time_set(uint8_t mab_time)
{
	DMXUSBPRO_params.mab_time = mab_time;
}

void widget_params_refresh_rate_set(uint8_t refresh_rate)
{
	DMXUSBPRO_params.refresh_rate = refresh_rate;
}

void widget_params_sn_get(struct _widget_sn *widget_sn)
{
	memcpy(widget_sn, &DMXUSBPRO_SN, sizeof(struct _widget_sn));
}

