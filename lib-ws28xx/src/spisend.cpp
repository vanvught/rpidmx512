/**
 * @file spisend.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ws28xx.h"

//#include "monitor.h"
#include "spisend.h"
#include "util.h"

/**
 *
 */
SPISend::SPISend(void) : m_led_type(WS2801), m_led_count(170) {
}

/**
 *
 */
SPISend::~SPISend(void)
{
	this->Stop();
}

/**
 *
 */
void SPISend::Start(void) {
	ws28xx_init(m_led_count, m_led_type, 0);
}

/**
 *
 */
void SPISend::Stop(void)
{

}

/**
 *
 * @param nPortId
 * @param data
 * @param length
 */
void SPISend::SetData(const uint8_t nPortId, const uint8_t *data, const uint16_t length)
{
	uint16_t i = 0;
	uint16_t j = 0;

	uint16_t beginIndex = (uint16_t) 0;
	uint16_t endIndex = (uint16_t) 0;

	bool bUpdate = false;

	switch (nPortId)
	{
	case 0:
		beginIndex = (uint16_t)0;
		endIndex = MIN(m_led_count, (uint16_t)(length / (uint16_t)3));
		bUpdate = (endIndex == m_led_count);
		break;
	case 1:
		beginIndex = (uint16_t)170;
		endIndex = MIN(m_led_count, (uint16_t)((uint16_t)170 + (length / (uint16_t)3)));
		bUpdate = (endIndex == m_led_count);
		break;
	case 2:
		beginIndex = (uint16_t)340;
		endIndex = MIN(m_led_count, (uint16_t)((uint16_t)340 + (length / (uint16_t)3)));
		bUpdate = (endIndex == m_led_count);
		break;
	case 3:
		beginIndex = (uint16_t)510;
		endIndex = MIN(m_led_count, (uint16_t)((uint16_t)510 + (length / (uint16_t)3)));
		bUpdate = (endIndex == m_led_count);
		break;
	default:
		break;
	}


	//monitor_line(MONITOR_LINE_STATS, "%d-%x:%x:%x-%d|%s", nPortId, data[0], data[1], data[2], length, bUpdate == false ? "False" : "True");

	for (j = beginIndex; j < endIndex; j++) {
		ws28xx_set_led(j, data[i], data[i + 1], data[i + 2]);
		i = i + 3;
	}

	if (bUpdate) {
		ws28xx_update();
	}
}

/**
 *
 * @param type
 */
void SPISend::SetLEDType(_ws28xxx_type type) {
	m_led_type = type;
}

/**
 *
 * @return
 */
const _ws28xxx_type SPISend::GetLEDType(void) {
	return m_led_type;
}

/**
 *
 * @param count
 */
void SPISend::SetLEDCount(const uint16_t count) {
	m_led_count = count;
}

/**
 *
 */
const uint16_t SPISend::GetLEDCount(void) {
	return m_led_count;
}
