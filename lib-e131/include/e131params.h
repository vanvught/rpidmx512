/**
 * @file e131params.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef E131PARAMS_H_
#define E131PARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "e131bridge.h"
#include "e131.h"

enum TOutputType {
	OUTPUT_TYPE_DMX,
	OUTPUT_TYPE_SPI,
	OUTPUT_TYPE_MONITOR
};

class E131Params {
public:
	E131Params(void);
	~E131Params(void);

	bool Load(void);

	TOutputType GetOutputType(void) const;
	uint16_t GetUniverse(void) const;
	TMerge GetMergeMode(void) const;
	bool isHaveCustomCid(void) const;
	const char *GetCidString(void);

	void Set(E131Bridge *);
	void Dump(void);

private:
	bool isMaskSet(uint32_t) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    uint32_t m_bSetList;

    uint16_t m_nUniverse;
    TOutputType m_tOutputType;
    TMerge m_tMergeMode;
    char m_aCidString[UUID_STRING_LENGTH + 2];
    bool m_bHaveCustomCid;
};

#endif /* E131PARAMS_H_ */
