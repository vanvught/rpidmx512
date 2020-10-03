/**
 * @file artnettrigger.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnettrigger.h"

#include "artnetnode.h"

#include "debug.h"

void ArtNetNode::HandleTrigger() {
	DEBUG_ENTRY
	const struct TArtTrigger *pArtTrigger = &(m_ArtNetPacket.ArtPacket.ArtTrigger);

	if ((pArtTrigger->OemCodeHi == 0xFF && pArtTrigger->OemCodeLo == 0xFF) || (pArtTrigger->OemCodeHi == m_Node.Oem[0] && pArtTrigger->OemCodeLo == m_Node.Oem[1])) {
		DEBUG_PRINTF("Key=%d, SubKey=%d, Data[0]=%d", pArtTrigger->Key, pArtTrigger->SubKey, pArtTrigger->Data[0]);

		m_pArtNetTrigger->Handler(reinterpret_cast<const struct TArtNetTrigger*>(&pArtTrigger->Key));
	}

	DEBUG_EXIT
}
