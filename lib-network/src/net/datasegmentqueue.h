/**
 * @file datasegmentqueue.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DATASEGMENTQUEUE_H_
#define DATASEGMENTQUEUE_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "net/protocol/tcp.h"

#if !defined(TCP_TX_QUEUE_SIZE)
# define TCP_TX_QUEUE_SIZE 	2
#endif

struct DataSegment {
	uint8_t buffer[TCP_DATA_SIZE];
	uint32_t nLength;
	bool isLastSegment;
};

class DataSegmentQueue {
public:
	bool IsEmpty() const {
		return !m_isFull && (m_nHead == m_nTail);
	}

	bool IsFull() const {
		return m_isFull;
	}

	bool Push(const uint8_t *pData, const uint32_t nLength, const bool isLastSegment) {
		assert(pData != nullptr);
		assert(nLength > 0);
		assert(nLength <= TCP_DATA_SIZE);

		if (IsFull() || (nLength > TCP_DATA_SIZE)) {
			return false;
		}

		auto& dataSegment = m_dataSegment[m_nHead];

		memcpy(dataSegment.buffer, pData, nLength);
		dataSegment.nLength = nLength;
		dataSegment.isLastSegment = isLastSegment;

		m_nHead = (m_nHead + 1) % TCP_TX_QUEUE_SIZE;
		m_isFull = (m_nHead == m_nTail);

		return true;
	}

	void Pop() {
		assert(!IsEmpty());

		if (IsEmpty()) {
			return;
		}

		m_nTail = (m_nTail + 1) % TCP_TX_QUEUE_SIZE;
		m_isFull = false;
	}

	const DataSegment& GetFront() const {
		return m_dataSegment[m_nTail];
	}

private:
	DataSegment m_dataSegment[TCP_TX_QUEUE_SIZE];
	uint32_t m_nHead { 0 };
	uint32_t m_nTail { 0 };
	bool m_isFull { false };
};

#endif /* DATASEGMENTQUEUE_H_ */
