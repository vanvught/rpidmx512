/**
 * @file artnettrigger.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETTRIGGER_H_
#define ARTNETTRIGGER_H_

#include <cstdint>

// When the Oem field = ffff the meaning of the SubKey field is defined by enum ArtTriggerKey

enum class ArtTriggerKey: uint8_t {
	ART_TRIGGER_KEY_ASCII,	///< The SubKey field contains an ASCII character which the receiving device should process as if it were a keyboard press. (Payload not used).
	ART_TRIGGER_KEY_MACRO,	///< The SubKey field contains the number of a Macro which the receiving device should execute. (Payload not used).
	ART_TRIGGER_KEY_SOFT,	///< The SubKey field contains a soft-key number which the receiving device should process as if it were a soft-key keyboard press. (Payload not used).
	ART_TRIGGER_KEY_SHOW,	///< The SubKey field contains the number of a Show which the receiving device should run. (Payload not used).
	ART_TRIGGER_UNDEFINED
};

inline bool operator== (uint8_t a, ArtTriggerKey b) {
	return (static_cast<uint32_t>(a) == static_cast<uint32_t>(b));
}

// If the Oem field is set to a value other than ffff then the Key and SubKey fields are manufacturer specific.

struct ArtNetTrigger {
	uint8_t Key;		///< The Trigger Key.
	uint8_t SubKey;		///< The Trigger SubKey.
	uint8_t Data[512];	///< The interpretation of the payload is defined by the Key.
}__attribute__((packed));

typedef void (*ArtTriggerCallbackFunctionPtr)(const struct ArtNetTrigger *);

#endif /* ARTNETTRIGGER_H_ */
