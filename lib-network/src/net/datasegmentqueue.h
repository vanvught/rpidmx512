/**
 * @file datasegmentqueue.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_DATASEGMENTQUEUE_H_
#define NET_DATASEGMENTQUEUE_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "net/protocol/tcp.h"

#if !defined(TCP_TX_QUEUE_SIZE)
#define TCP_TX_QUEUE_SIZE 2
#endif

struct DataSegment
{
    uint8_t buffer[TCP_DATA_SIZE];
    uint32_t length;
    bool is_last_segment;
};

class DataSegmentQueue
{
   public:
    DataSegmentQueue() = default;
    DataSegmentQueue(const DataSegmentQueue&) = delete;
    DataSegmentQueue& operator=(const DataSegmentQueue&) = delete;
    DataSegmentQueue(DataSegmentQueue&&) = delete;
    DataSegmentQueue& operator=(DataSegmentQueue&&) = delete;

    bool IsEmpty() const { return !full_ && (head_ == tail_); }

    bool IsFull() const { return full_; }

    bool Push(const uint8_t* data, uint32_t length, bool is_last_segment)
    {
        assert(data != nullptr);
        assert(length > 0);
        assert(length <= TCP_DATA_SIZE);

        if (IsFull() || (length > TCP_DATA_SIZE))
        {
            return false;
        }

        auto& data_segment = data_segment_[head_];

        memcpy(data_segment.buffer, data, length);
        data_segment.length = length;
        data_segment.is_last_segment = is_last_segment;

        head_ = (head_ + 1) % TCP_TX_QUEUE_SIZE;
        full_ = (head_ == tail_);

        return true;
    }

    void Pop()
    {
        assert(!IsEmpty());

        if (IsEmpty())
        {
            return;
        }

        tail_ = (tail_ + 1) % TCP_TX_QUEUE_SIZE;
        full_ = false;
    }

    const DataSegment& GetFront() const { return data_segment_[tail_]; }

   private:
    DataSegment data_segment_[TCP_TX_QUEUE_SIZE];
    uint32_t head_{0};
    uint32_t tail_{0};
    bool full_{false};
};

#endif  // NET_DATASEGMENTQUEUE_H_
