/**
 * @file storewidget.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "storewidget.h"

#include "widgetparams.h"

#include "spiflashstore.h"

#include "debug.h"

StoreWidget *StoreWidget::s_pThis = nullptr;

StoreWidget::StoreWidget() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("%p", reinterpret_cast<void *>(s_pThis));
	DEBUG_EXIT
}

void StoreWidget::Update(const struct TWidgetParams* pWidgetParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_WIDGET, pWidgetParams, sizeof(struct TWidgetParams));

	DEBUG_EXIT
}

void StoreWidget::Copy(struct TWidgetParams* pWidgetParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Copy(STORE_WIDGET, pWidgetParams, sizeof(struct TWidgetParams));

	DEBUG_EXIT
}

void StoreWidget::UpdateBreakTime(uint8_t nBreakTime) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TWidgetParams, nBreakTime));

	SpiFlashStore::Get()->Update(STORE_WIDGET, __builtin_offsetof(struct TWidgetParams, nBreakTime), &nBreakTime, sizeof(uint8_t), WIDGET_PARAMS_MASK_BREAK_TIME);

	DEBUG_EXIT
}

void StoreWidget::UpdateMabTime(uint8_t nMabTime) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TWidgetParams, nMabTime));

	SpiFlashStore::Get()->Update(STORE_WIDGET, __builtin_offsetof(struct TWidgetParams, nMabTime), &nMabTime, sizeof(uint8_t), WIDGET_PARAMS_MASK_MAB_TIME);

	DEBUG_EXIT
}

void StoreWidget::UpdateRefreshRate(uint8_t nRefreshRate) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TWidgetParams, nRefreshRate));

	SpiFlashStore::Get()->Update(STORE_WIDGET, __builtin_offsetof(struct TWidgetParams, nRefreshRate), &nRefreshRate, sizeof(uint8_t), WIDGET_PARAMS_MASK_REFRESH_RATE);

	DEBUG_EXIT
}
