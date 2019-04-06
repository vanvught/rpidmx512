/**
 * @file widget_params.cpp
 *
 * @brief Bridge between C++ and C
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

#include <stdint.h>
#include <assert.h>

#include "widget_params.h"

#include "widgetstore.h"

#include "dmx.h"

static const uint8_t DEVICE_TYPE_ID[DEVICE_TYPE_ID_LENGTH] __attribute__((aligned(4))) = { (uint8_t) 1, (uint8_t) 0 };

static struct _widget_params dmx_usb_pro_params __attribute__((aligned(4))) = {
		(uint8_t) WIDGET_DEFAULT_FIRMWARE_LSB, (uint8_t) FIRMWARE_RDM,
		(uint8_t) WIDGET_DEFAULT_BREAK_TIME, (uint8_t) WIDGET_DEFAULT_MAB_TIME,
		(uint8_t) WIDGET_DEFAULT_REFRESH_RATE };

static WidgetStore *spWidgetStore = 0;

void widget_params_set_store(WidgetStore *pWidgetStore) {
	spWidgetStore = pWidgetStore;
}

extern "C" {
void widget_params_set_firmware_msb(uint8_t msb) {
	dmx_usb_pro_params.firmware_msb = msb;
}
void widget_params_set_break_time(uint8_t break_time) {
	dmx_usb_pro_params.break_time = break_time;
}
void widget_params_set_mab_time(uint8_t mab_time) {
	dmx_usb_pro_params.mab_time = mab_time;
}
void widget_params_set_refresh_rate(uint8_t refresh_rate) {
	dmx_usb_pro_params.refresh_rate = refresh_rate;
}
}

void widget_params_get_type_id(struct _widget_params_data *info) {
	info->data = (uint8_t *) DEVICE_TYPE_ID;
	info->length = (uint8_t) DEVICE_TYPE_ID_LENGTH;
}

void widget_params_get(struct _widget_params *widget_params) {
	widget_params->break_time = dmx_usb_pro_params.break_time;
	widget_params->firmware_lsb = dmx_usb_pro_params.firmware_lsb;
	widget_params->firmware_msb = dmx_usb_pro_params.firmware_msb;
	widget_params->mab_time = dmx_usb_pro_params.mab_time;
	widget_params->refresh_rate = dmx_usb_pro_params.refresh_rate;
}

void widget_params_set(const struct _widget_params *widget_params) {
	assert(spWidgetStore != 0);

	if (widget_params->break_time != dmx_usb_pro_params.break_time) {
		dmx_usb_pro_params.break_time = widget_params->break_time;
		dmx_set_output_break_time((uint32_t) ((double) (dmx_usb_pro_params.break_time) * (double) (10.67)));

		spWidgetStore->UpdateBreakTime(widget_params->break_time);
	}

	if (widget_params->mab_time != dmx_usb_pro_params.mab_time) {
		dmx_usb_pro_params.mab_time = widget_params->mab_time;
		dmx_set_output_mab_time((uint32_t) ((double) (dmx_usb_pro_params.mab_time) * (double) (10.67)));

		spWidgetStore->UpdateMabTime(widget_params->mab_time);
	}

	if (widget_params->refresh_rate != dmx_usb_pro_params.refresh_rate) {
		dmx_usb_pro_params.refresh_rate = widget_params->refresh_rate;
		dmx_set_output_period(widget_params->refresh_rate == (uint8_t) 0 ? (uint32_t) 0 : (uint32_t) (1000000 / widget_params->refresh_rate));

		spWidgetStore->UpdateRefreshRate(widget_params->refresh_rate);
	}
}
