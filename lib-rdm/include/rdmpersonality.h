/**
 * @file rdmpersonality.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RDMPERSONALITY_H_
#define RDMPERSONALITY_H_

#define RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH		32

class RDMPersonality {
public:
	RDMPersonality(const char *pDescription, uint16_t nSlots);
	~RDMPersonality(void);

	uint16_t GetSlots(void) const;

	const char *GetDescription(void) const;
	void SetDescription(const char *pDescription);

	uint8_t GetDescriptionLength(void) const;

	void CopyTo(char* p, uint8_t &nLength);

private:
	uint16_t m_nSlots;
	char m_aDescription[RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH];
	uint8_t m_nDescriptionLength;
};

#endif /* RDMPERSONALITY_H_ */
