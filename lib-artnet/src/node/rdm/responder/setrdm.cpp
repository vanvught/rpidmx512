/**
 * @file setrdm.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnetnode.h"
#include "artnetrdmresponder.h"

#include "debug.h"

void ArtNetNode::SetRdm(const bool doEnable) {
	DEBUG_ENTRY

	SetRdmResponder(m_pArtNetRdmResponder, doEnable);

	DEBUG_EXIT
}

void ArtNetNode::SetRdmResponder(ArtNetRdmResponder *pArtNetRdmResponder, const bool doEnable) {
	DEBUG_ENTRY

	m_pArtNetRdmResponder = pArtNetRdmResponder;
	m_State.rdm.IsEnabled = ((pArtNetRdmResponder != nullptr) & doEnable);

	if (m_State.rdm.IsEnabled) {
		m_ArtPollReply.Status1 |= artnet::Status1::RDM_CAPABLE;
	} else {
		m_ArtPollReply.Status1 &= static_cast<uint8_t>(~artnet::Status1::RDM_CAPABLE);
	}

	DEBUG_PRINTF("m_State.rdm.IsEnabled=%c", m_State.rdm.IsEnabled ? 'Y' : 'N');
	DEBUG_EXIT
}
