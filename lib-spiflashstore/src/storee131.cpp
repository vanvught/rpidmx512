/**
 * @file storee131.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "spiflashstore.h"

#include "e131params.h"

#include "debug.h"

E131ParamsStore::~E131ParamsStore(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

StoreE131::StoreE131(void) {
	DEBUG_ENTRY

	DEBUG_PRINTF("%p", this);

	DEBUG_EXIT
}

StoreE131::~StoreE131(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void StoreE131::Update(const struct TE131Params* pE131Params) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_E131, (void *)pE131Params, sizeof(struct TE131Params));

	DEBUG_EXIT
}

void StoreE131::Copy(struct TE131Params* pE131Params) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Copy(STORE_E131, (void *)pE131Params, sizeof(struct TE131Params));

	DEBUG_EXIT
}
