/**
 * @file rdmdevice.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RDMDEVICE_H_
#define RDMDEVICE_H_

#include <stdint.h>

struct _rdm_device_info_data {
	/*@shared@*//*@null@*/uint8_t *data;
	uint8_t length;
};

#include "rdm.h"

#define DEVICE_SN_LENGTH		4	///<

class RDMDevice {
public:
	RDMDevice(void);
	~RDMDevice(void);

	void ReadConfigFile(void);

	const uint8_t *GetUID(void);

	const uint8_t GetExtMonLevel(void);

	void SetLabel(struct _rdm_device_info_data *);
	void GetLabel(struct _rdm_device_info_data *);

	void SetManufacturerId(struct _rdm_device_info_data *);
	void GetManufacturerId(struct _rdm_device_info_data *);

	void SetManufacturerName(struct _rdm_device_info_data *);
	void GetManufacturerName(struct _rdm_device_info_data *);
};

#endif /* RDMDEVICE_H_ */
