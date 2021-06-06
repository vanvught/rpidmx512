/**
 * @file widgetusb.cpp
 *
 */
/* Copyright (C) 2015-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "widget.h"
#include "usb.h"

using namespace widget;

void Widget::SendHeader(uint8_t nLabel, uint32_t nLength) {
	usb_send_byte(static_cast<uint8_t>(Amf::START_CODE));
	usb_send_byte(nLabel);
	usb_send_byte(static_cast<uint8_t>(nLength & 0x00FF));
	usb_send_byte(static_cast<uint8_t>(nLength >> 8));
}

void Widget::SendData(const uint8_t *pData, uint32_t nLength) {
	for (uint32_t i = 0; i < nLength; i++) {
		usb_send_byte(pData[i]);
	}
}

void Widget::SendFooter() {
	usb_send_byte(static_cast<uint8_t>(Amf::END_CODE));
}

void Widget::SendMessage(uint8_t nLabel, const uint8_t *pData, uint32_t nLength) {
	SendHeader(nLabel, nLength);
	SendData(pData, nLength);
	SendFooter();
}
