/**
 * @file showfileprotocolartnettrigger.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "protocols/showfileprotocolartnettrigger.h"
#include "showfile.h"

#include "debug.h"

void ShowFileProtocolArtNetTrigger::Handler(const struct TArtNetTrigger *ptArtNetTrigger)  {
	DEBUG_ENTRY
	DEBUG_PRINTF("Key=%d, SubKey=%d", ptArtNetTrigger->Key, ptArtNetTrigger->SubKey);

	if (ptArtNetTrigger->Key == ART_TRIGGER_KEY_SOFT) {
		switch (ptArtNetTrigger->SubKey) {
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

	if (ptArtNetTrigger->Key == ART_TRIGGER_KEY_SHOW) {
		ShowFile::Get()->SetPlayerShowFileCurrent(ptArtNetTrigger->SubKey);
	}

	DEBUG_EXIT
}
