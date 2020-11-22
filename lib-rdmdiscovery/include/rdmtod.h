/**
 * @file rdmtod.h
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMTOD_H_
#define RDMTOD_H_

#include <stdint.h>

#include "rdm.h"

#define TOD_TABLE_SIZE	200

struct TRdmTod {
	uint8_t uid[RDM_UID_SIZE];
};

class RDMTod {
public:
	 RDMTod();
	 ~RDMTod();

	 void Reset();
	 bool AddUid(const uint8_t *pUid);
	 uint8_t GetUidCount() const;
	 void Copy(uint8_t *pTable);

	 bool Delete(const uint8_t *pUid);
	 bool Exist(const uint8_t *pUid);

	 void Dump();
	 void Dump(uint8_t nCount);
private:
	 uint8_t m_nEntries{0};
	 TRdmTod *m_pTable;
};

#endif /* RDMTOD_H_ */
