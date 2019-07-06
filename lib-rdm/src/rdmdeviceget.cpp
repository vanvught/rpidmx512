/**
 * @file rdmdeviceget.cpp
 *
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

#include "rdmdevice.h"

void RDMDevice::GetLabel(struct TRDMDeviceInfoData *info) {
	info->data = (uint8_t *) m_tRDMDeviceParams.aDeviceRootLabel;
	info->length = m_tRDMDeviceParams.nDeviceRootLabelLength;
}

void RDMDevice::GetManufacturerId(struct TRDMDeviceInfoData *info) {
	info->data[0] = m_tRDMDeviceParams.aDeviceUID[1];
	info->data[1] = m_tRDMDeviceParams.aDeviceUID[0];
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void RDMDevice::GetManufacturerName(struct TRDMDeviceInfoData *info) {
	info->data = (uint8_t *) m_tRDMDeviceParams.aDeviceManufacturerName;
	info->length = m_tRDMDeviceParams.nDdeviceManufacturerNameLength;
}
