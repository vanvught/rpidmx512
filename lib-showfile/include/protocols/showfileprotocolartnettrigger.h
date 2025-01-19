/**
 * @file showfileprotocolartnettrigger.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLARTNETTRIGGER_H_
#define PROTOCOLS_SHOWFILEPROTOCOLARTNETTRIGGER_H_

#include <cstdint>
#include <cstdio>

#include "artnetcontroller.h"
#include "artnettrigger.h"

#include "debug.h"

class ShowFileProtocolArtNetTrigger {
public:
	ShowFileProtocolArtNetTrigger() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		ArtNetController::Get()->SetArtTriggerCallbackFunctionPtr(StaticCallbackFunction);

		DEBUG_EXIT
	}

private:
	void Handler(const struct ArtNetTrigger *pArtNetTrigger)  {
		DEBUG_ENTRY
		DEBUG_PRINTF("Key=%d, SubKey=%d", pArtNetTrigger->Key, pArtNetTrigger->SubKey);

		if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_KEY_SOFT) {
			switch (pArtNetTrigger->SubKey) {
			case 'B':
				ShowFile::Get()->BlackOut();
				break;
			case 'G':
				ShowFile::Get()->Play();
				break;
			case 'R':
				ShowFile::Get()->Resume();
				break;
			case 'S':
				ShowFile::Get()->Stop();
				break;
			default:
				break;
			}
		}

		if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_KEY_SHOW) {
			ShowFile::Get()->SetPlayerShowFileCurrent(pArtNetTrigger->SubKey);
		}

		DEBUG_EXIT
	}

private:
	static inline ShowFileProtocolArtNetTrigger *s_pThis;
};

#endif /* PROTOCOLS_SHOWFILEPROTOCOLARTNETTRIGGER_H_ */
