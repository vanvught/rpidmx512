/**
 * @file widgetconfiguration.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>

#include "widgetconfiguration.h"
#include "widgetparamsconst.h"
#include "sscan.h"
#include "../ff14b/source/ff.h"

typedef enum {
	kAiBreakTime = 0,
	kAiMabTime,
	kAiRefreshRate
} _array_index;

static bool needs_update[3] = { false, false, false };

static const TCHAR kFileNameParams[] = "params.txt";
static const TCHAR kFileNameParamsBak[] = "params.bak";
static const TCHAR kFileNameUpdates[] = "updates.txt";

static char* Uint8Toa(uint8_t i) {
	/* Room for 3 digits and '\0' */
	static char buffer[4];
	char *p = &buffer[3]; /* points to terminating '\0' */

	*p = '\0';

	do {
		*--p = '0' + static_cast<char>(i % 10);
		i /= 10;
	} while (i != 0);

	return p;
}

static void SprintfNameValue(char *buffer, const char *name, uint8_t value) {
	auto *dst = buffer;
	const auto *src = name;

	while (*src != '\0') {
		*dst++ = *src++;
	}

	*dst++ = '=';

	auto *p = Uint8Toa(value);

	while (*p != '\0') {
		*dst++ = *p++;
	}

	*dst++ = '\n';
	*dst = '\0';
}

void WidgetConfiguration::ProcessLineUpdate(const char* line, FIL* file_object_wr)
{
    TCHAR buffer[128];
	uint8_t value;
	int i;

	if (needs_update[kAiBreakTime]) {
		if (Sscan::Uint8(line, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, value) == Sscan::OK) {
			SprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, s_break_time);
			f_puts(buffer, file_object_wr);
			needs_update[kAiBreakTime] = false;
			return;
		}
	}

	if (needs_update[kAiMabTime]) {
		if (Sscan::Uint8(line, WidgetParamsConst::DMXUSBPRO_MAB_TIME, value) == Sscan::OK) {
			SprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_MAB_TIME, s_mab_time);
			f_puts(buffer, file_object_wr);
			needs_update[kAiMabTime] = false;
			return;
		}
	}

	if (needs_update[kAiRefreshRate]) {
		if (Sscan::Uint8(line, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, value) == Sscan::OK) {
			SprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, s_refresh_rate);
			f_puts(buffer, file_object_wr);
			needs_update[kAiRefreshRate] = false;
			return;
		}
	}

	f_puts(line, file_object_wr);
	i = static_cast<int>(strlen(line)) - 1;
	if (line[i] != '\n') {
		f_putc(static_cast<TCHAR>('\n'), file_object_wr);
	}
}

void WidgetConfiguration::UpdateConfigFile() {
	TCHAR buffer[128];
	FIL file_object_rd;
	FIL file_object_wr;
	FRESULT rc_rd = FR_DISK_ERR;
	FRESULT rc_wr = FR_DISK_ERR;

	rc_rd = f_open(&file_object_rd, kFileNameParams, (BYTE) FA_READ);
	rc_wr = f_open(&file_object_wr, kFileNameUpdates, static_cast<BYTE>(FA_WRITE | FA_CREATE_ALWAYS));

	if (rc_wr == FR_OK) {

		if (rc_rd == FR_OK) {
			for (;;) {
				if (f_gets(buffer, static_cast<int>(sizeof(buffer)), &file_object_rd) == nullptr) {
					break; // Error or end of file
				}
				ProcessLineUpdate((const char *) buffer, &file_object_wr);
			}
		}

		if (needs_update[kAiBreakTime]) {
			SprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, s_break_time);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[kAiBreakTime] = false;
		}

		if (needs_update[kAiMabTime]) {
			SprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_MAB_TIME, s_mab_time);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[kAiMabTime] = false;
		}

		if (needs_update[kAiRefreshRate]) {
			SprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, s_refresh_rate);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[kAiRefreshRate] = false;
		}

		f_close(&file_object_wr);

		if (rc_rd == FR_OK) {
			(void) f_close(&file_object_rd);
		}

		f_unlink(kFileNameParamsBak);
		f_rename(kFileNameParams, kFileNameParamsBak);
		f_rename(kFileNameUpdates, kFileNameParams);
	}
}

void WidgetConfiguration::Store(const struct TWidgetConfiguration *widget_params) {
	bool call_update_config_file = false;

	if (widget_params->break_time != s_break_time) {
		s_break_time = widget_params->break_time;
		Dmx::Get()->SetDmxBreakTime((static_cast<float>((s_break_time)) * 10.67));
		needs_update[kAiBreakTime] = true;
		call_update_config_file = true;
	}

	if (widget_params->mab_time != s_mab_time) {
		s_mab_time = widget_params->mab_time;
		Dmx::Get()->SetDmxMabTime((static_cast<float>((s_mab_time)) * 10.67));
		needs_update[kAiMabTime] = true;
		call_update_config_file = true;
	}

	if (widget_params->refresh_rate != s_refresh_rate) {
		s_refresh_rate = widget_params->refresh_rate;
		Dmx::Get()->SetDmxPeriodTime(widget_params->refresh_rate == 0 ? 0 : (1000000U / widget_params->refresh_rate));
		needs_update[kAiRefreshRate] = true;
		call_update_config_file = true;
	}

	if (call_update_config_file) {
		UpdateConfigFile();
	}
}
