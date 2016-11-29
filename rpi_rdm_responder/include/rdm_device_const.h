/**
 * @file rdm_device_const.h
 *
 * @brief These definitions are private for the RDM Responder
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

#ifndef RDM_DEVICE_CONST_H_
#define RDM_DEVICE_CONST_H_

#include <stdint.h>

#include "util.h"

#include "rdm.h"
#include "sofware_version_id.h"

static const char DEVICE_LABEL[] ALIGNED = "Raspberry Pi";				///<
static const char DEVICE_MANUFACTURER_NAME[] ALIGNED = "AvV";			///<
static const uint8_t DEVICE_MANUFACTURER_ID[] ALIGNED = { 0x7F, 0xF0 };	///< 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static const char DEVICE_SUPPORTED_LANGUAGE[] ALIGNED = "en";			///<
static const char DEVICE_SOFTWARE_VERSION[] ALIGNED = "1.3";			///<

#define DEFAULT_DMX_START_ADDRESS		1	///<
#define DEFAULT_CURRENT_PERSONALITY		1	///<
#define DMX_FOOTPRINT					32	///<

static const struct _rdm_personality rdm_personalities[] ALIGNED = {
		{ (uint16_t)DMX_FOOTPRINT, "RDM Responder / DMX Controller", (uint8_t)30 }
		};

#endif /* RDM_DEVICE_CONST_H_ */
