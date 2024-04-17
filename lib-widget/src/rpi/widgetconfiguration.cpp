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
#include <cstddef>
#include <cstring>
#include <cassert>

#include "widgetconfiguration.h"
#include "widgetparamsconst.h"

#include "sscan.h"

#include "../ff14b/source/ff.h"

#include "dmx.h"

typedef enum {
	AI_BREAK_TIME = 0,
	AI_MAB_TIME,
	AI_REFRESH_RATE
} _array_index;

static bool needs_update[3] = { false, false, false };

static const TCHAR FILE_NAME_PARAMS[] = "params.txt";
static const TCHAR FILE_NAME_PARAMS_BAK[] = "params.bak";
static const TCHAR FILE_NAME_UPDATES[] = "updates.txt";

static char* uint8_toa(uint8_t i) {
	/* Room for 3 digits and '\0' */
	static char buffer[4];
	char *p = &buffer[3]; /* points to terminating '\0' */

	*p = '\0';

	do {
		*--p = (char) '0' + (char) (i % 10);
		i /= 10;
	} while (i != 0);

	return p;
}

static void sprintfNameValue(char *pBuffer, const char *pName, const uint8_t nValue) {
	auto *pDst = pBuffer;
	const auto *pSrc = pName;

	while (*pSrc != '\0') {
		*pDst++ = *pSrc++;
	}

	*pDst++ = '=';

	auto *p = uint8_toa(nValue);

	while (*p != '\0') {
		*pDst++ = *p++;
	}

	*pDst++ = '\n';
	*pDst = '\0';
}

void WidgetConfiguration::ProcessLineUpdate(const char *pLine, FIL *file_object_wr) {
	TCHAR buffer[128];
	uint8_t _value;
	int i;

	if (needs_update[AI_BREAK_TIME]) {
		if (Sscan::Uint8(pLine, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, _value) == Sscan::OK) {
			sprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, s_nBreakTime);
			f_puts(buffer, file_object_wr);
			needs_update[AI_BREAK_TIME] = false;
			return;
		}
	}

	if (needs_update[AI_MAB_TIME]) {
		if (Sscan::Uint8(pLine, WidgetParamsConst::DMXUSBPRO_MAB_TIME, _value) == Sscan::OK) {
			sprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_MAB_TIME, s_nMabTime);
			f_puts(buffer, file_object_wr);
			needs_update[AI_MAB_TIME] = false;
			return;
		}
	}

	if (needs_update[AI_REFRESH_RATE]) {
		if (Sscan::Uint8(pLine, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, _value) == Sscan::OK) {
			sprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, s_nRefreshRate);
			f_puts(buffer, file_object_wr);
			needs_update[AI_REFRESH_RATE] = false;
			return;
		}
	}

	f_puts(pLine, file_object_wr);
	i = (int) strlen(pLine) - 1;
	if (pLine[i] != (char) '\n') {
		f_putc((TCHAR) '\n', file_object_wr);
	}

}

void WidgetConfiguration::UpdateConfigFile() {
	TCHAR buffer[128];
	FIL file_object_rd;
	FIL file_object_wr;
	FRESULT rc_rd = FR_DISK_ERR;
	FRESULT rc_wr = FR_DISK_ERR;

	rc_rd = f_open(&file_object_rd, FILE_NAME_PARAMS, (BYTE) FA_READ);
	rc_wr = f_open(&file_object_wr, FILE_NAME_UPDATES, (BYTE) (FA_WRITE | FA_CREATE_ALWAYS));

	if (rc_wr == FR_OK) {

		if (rc_rd == FR_OK) {
			for (;;) {
				if (f_gets(buffer, (int) sizeof(buffer), &file_object_rd) == nullptr) {
					break; // Error or end of file
				}
				ProcessLineUpdate((const char *) buffer, &file_object_wr);
			}
		}

		if (needs_update[AI_BREAK_TIME]) {
			sprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, s_nBreakTime);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[AI_BREAK_TIME] = false;
		}

		if (needs_update[AI_MAB_TIME]) {
			sprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_MAB_TIME, s_nMabTime);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[AI_MAB_TIME] = false;
		}

		if (needs_update[AI_REFRESH_RATE]) {
			sprintfNameValue(buffer, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, s_nRefreshRate);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[AI_REFRESH_RATE] = false;
		}

		f_close(&file_object_wr);

		if (rc_rd == FR_OK) {
			(void) f_close(&file_object_rd);
		}

		f_unlink(FILE_NAME_PARAMS_BAK);
		f_rename(FILE_NAME_PARAMS, FILE_NAME_PARAMS_BAK);
		f_rename(FILE_NAME_UPDATES, FILE_NAME_PARAMS);
	}
}

void WidgetConfiguration::Store(const struct TWidgetConfiguration *widget_params) {
	bool call_update_config_file = false;

	if (widget_params->nBreakTime != s_nBreakTime) {
		s_nBreakTime = widget_params->nBreakTime;
		Dmx::Get()->SetDmxBreakTime((static_cast<float>((s_nBreakTime)) * 10.67));
		needs_update[AI_BREAK_TIME] = true;
		call_update_config_file = true;
	}

	if (widget_params->nMabTime != s_nMabTime) {
		s_nMabTime = widget_params->nMabTime;
		Dmx::Get()->SetDmxMabTime((static_cast<float>((s_nMabTime)) * 10.67));
		needs_update[AI_MAB_TIME] = true;
		call_update_config_file = true;
	}

	if (widget_params->nRefreshRate != s_nRefreshRate) {
		s_nRefreshRate = widget_params->nRefreshRate;
		Dmx::Get()->SetDmxPeriodTime(widget_params->nRefreshRate == 0 ? 0 : (1000000U / widget_params->nRefreshRate));
		needs_update[AI_REFRESH_RATE] = true;
		call_update_config_file = true;
	}

	if (call_update_config_file) {
		UpdateConfigFile();
	}
}
