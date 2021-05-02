/**
 * @file rdmpersonality.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <cassert>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "rdmpersonality.h"

RDMPersonality::RDMPersonality(const char* pDescription, uint16_t nSlots):
	m_nSlots(nSlots),
	m_nDescriptionLength(0)
{
	assert(pDescription != nullptr);

	SetDescription(pDescription);
}

uint16_t RDMPersonality::GetSlots() const {
	return m_nSlots;
}

const char* RDMPersonality::GetDescription() const {
	return m_aDescription;
}

void RDMPersonality::SetDescription(const char *pDescription) {
	assert(pDescription != nullptr);

	m_nDescriptionLength = 0;

	const char *pSrc = pDescription;
	char *pDst = m_aDescription;

	for (unsigned i = 0; (*pSrc != 0) && (i < RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH); i++) {
		*pDst = *pSrc;
		pSrc++;
		pDst++;
		m_nDescriptionLength++;
	}

}

uint8_t RDMPersonality::GetDescriptionLength() const {
	return m_nDescriptionLength;
}

void RDMPersonality::DescriptionCopyTo(char *p, uint8_t &nLength) {
	assert(p != nullptr);

	char *pSrc = m_aDescription;
	char *pDst = p;
	uint32_t i;

	for (i = 0; (i < m_nDescriptionLength) && (i < nLength); i++) {
		*pDst = *pSrc;
		pSrc++;
		pDst++;
	}

	nLength = i;
}
