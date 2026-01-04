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
 #include "firmware/debug/debug_debug.h"

void ArtNetNode::SetRdm(bool do_enable) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("do_enable=%u", static_cast<uint32_t>(do_enable));

	SetRdmResponder(rdm_responder_, do_enable);

	DEBUG_EXIT();
}

void ArtNetNode::SetRdmResponder(ArtNetRdmResponder *pArtNetRdmResponder, bool do_enable) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("do_enable=%u", static_cast<uint32_t>(do_enable));

	rdm_responder_ = pArtNetRdmResponder;
	state_.is_rdm_enabled = ((pArtNetRdmResponder != nullptr) & do_enable);

	if (state_.is_rdm_enabled) {
		art_poll_reply_.Status1 |= artnet::Status1::kRdmCapable;
	} else {
		art_poll_reply_.Status1 &= static_cast<uint8_t>(~artnet::Status1::kRdmCapable);
	}

	DEBUG_PRINTF("state_.is_rdm_enabled=%c", state_.is_rdm_enabled ? 'Y' : 'N');
	DEBUG_EXIT();
}
