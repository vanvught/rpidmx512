/**
 * @file dmxnode_data.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODE_DATA_H_
#define DMXNODE_DATA_H_

#include <cstdint>
#include <cassert>

#include "dmxnode_outputtype.h"
#include "dmxnodedata.h"

namespace dmxnode {
inline void data_set(DmxNodeOutputType *const pDmxNodeOutputType, const uint32_t nPortIndex) {
	assert(pDmxNodeOutputType != nullptr);

	pDmxNodeOutputType->SetData(nPortIndex, dmxnode::Data::Backup(nPortIndex), dmxnode::Data::GetLength(nPortIndex), false);
}

inline void data_output(DmxNodeOutputType *const pDmxNodeOutputType, const uint32_t nPortIndex) {
	assert(pDmxNodeOutputType != nullptr);

	pDmxNodeOutputType->SetData(nPortIndex, dmxnode::Data::Backup(nPortIndex), dmxnode::Data::GetLength(nPortIndex), true);
}
}  // namespace dmxnode

#endif /* DMXNODE_DATA_H_ */
