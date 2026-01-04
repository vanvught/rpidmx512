/**
 * @file rdmpersonality.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMPERSONALITY_H_
#define RDMPERSONALITY_H_

#include <cstdint>
#include <cassert>

#include "dmxnode_outputtype.h"
 #include "firmware/debug/debug_debug.h"

namespace rdm::personality {
static constexpr auto DESCRIPTION_MAX_LENGTH = 32U;
} // namespace rdm::personality


class RDMPersonality {
public:
	RDMPersonality(const char *pDescription, DmxNodeOutputType *pDmxNodeOutputType) {
		assert(pDescription != nullptr);

		if (pDmxNodeOutputType == nullptr) {
			slots_ = 0;
		} else {
			slots_ = pDmxNodeOutputType->GetDmxFootprint();
			dmxnode_output_type_ = pDmxNodeOutputType;
		}

		SetDescription(pDescription);
	}

	RDMPersonality(const char* pDescription, uint16_t nSlots): slots_(nSlots) {
		DEBUG_ENTRY();
		assert(pDescription != nullptr);

		SetDescription(pDescription);

		DEBUG_EXIT();
	}

	uint16_t GetSlots() const {
		return slots_;
	}

	DmxNodeOutputType *GetDmxNodeOutputType() const {
		return dmxnode_output_type_;
	}

	void SetDescription(const char *description) {
		assert(description != nullptr);

		description_length_ = 0;

		const auto *pSrc = description;
		auto *pDst = description_;

		for (uint32_t i = 0; (*pSrc != 0) && (i < rdm::personality::DESCRIPTION_MAX_LENGTH); i++) {
			*pDst = *pSrc;
			pSrc++;
			pDst++;
			description_length_++;
		}
	}

	const char *GetDescription() const {
		return description_;
	}

	uint8_t GetDescriptionLength() const {
		return static_cast<uint8_t>(description_length_);
	}

	void DescriptionCopyTo(char* p, uint8_t &nLength) {
		assert(p != nullptr);

		const auto *pSrc = description_;
		auto *pDst = p;
		uint8_t i;

		for (i = 0; (i < description_length_) && (i < nLength); i++) {
			*pDst = *pSrc;
			pSrc++;
			pDst++;
		}

		nLength = i;
	}

private:
	uint16_t slots_;
	DmxNodeOutputType *dmxnode_output_type_ { nullptr };
	char description_[rdm::personality::DESCRIPTION_MAX_LENGTH];
	uint32_t description_length_ { 0 };
};

#endif  // RDMPERSONALITY_H_
