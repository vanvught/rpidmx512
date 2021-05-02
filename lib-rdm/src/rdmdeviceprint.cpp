/**
 * @file rdmdeviceprint.cpp
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

#include <stdio.h>

#include "rdmdevice.h"

void RDMDevice::Print() {
	printf("RDM Device configuration\n");
	printf(" Manufacturer Name : %.*s\n", m_tRDMDevice.nDdeviceManufacturerNameLength, m_tRDMDevice.aDeviceManufacturerName);
	printf(" Manufacturer ID   : %.2X%.2X\n", m_tRDMDevice.aDeviceUID[0], m_tRDMDevice.aDeviceUID[1]);
	printf(" Serial Number     : %.2X%.2X%.2X%.2X\n", m_tRDMDevice.aDeviceSN[3], m_tRDMDevice.aDeviceSN[2], m_tRDMDevice.aDeviceSN[1], m_tRDMDevice.aDeviceSN[0]);
	printf(" Root label        : %.*s\n", m_tRDMDevice.nDeviceRootLabelLength, m_tRDMDevice.aDeviceRootLabel);
	printf(" Product Category  : %.2X%.2X\n", m_tRDMDevice.nProductCategory >> 8, m_tRDMDevice.nProductCategory & 0xFF);
	printf(" Product Detail    : %.2X%.2X\n", m_tRDMDevice.nProductDetail >> 8, m_tRDMDevice.nProductDetail & 0xFF);
}
