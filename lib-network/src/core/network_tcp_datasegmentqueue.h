/**
 * @file network_tcp_datasegmentqueue.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NETWORK_TCP_DATASEGMENTQUEUE_H_
#define NETWORK_TCP_DATASEGMENTQUEUE_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "core/protocol/tcp.h"
#include "network_memory.h"

#define TCP_TX_QUEUE_SIZE 8

namespace network::tcp::datasegment
{
struct NodeData
{
    uint8_t buffer[kTcpDataMss];
    uint32_t length;
    bool is_last_segment;
};

struct Node
{
    NodeData node_data;
    Node* next;
};

static_assert(sizeof(Node) <= network::memory::kBlockSize);

class Queue
{
   public:
    Queue() = default;
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    Queue(Queue&&) = delete;
    Queue& operator=(Queue&&) = delete;

    bool IsEmpty() const { return !full_ && (front_ == nullptr); }

    bool IsFull() const { return full_; }

    bool Push(const uint8_t* data, uint32_t length, bool is_last_segment)
    {
        assert(data != nullptr);
        assert(length > 0);
        assert(length <= kTcpDataMss);

        if (IsFull() || (length > kTcpDataMss)) [[unlikely]]
        {
            return false;
        }

        auto* add = reinterpret_cast<Node *>(memory::Allocator::Instance().Allocate());

        full_ = (add == nullptr);

        if (full_) [[unlikely]]
        {
            return false;
        }

        auto& data_segment = add->node_data;

        memcpy(data_segment.buffer, data, length);
        data_segment.length = length;
        data_segment.is_last_segment = is_last_segment;

        if (front_ == nullptr)
        {
            assert(last_ == nullptr);

            front_ = add;
            front_->next = nullptr;
            last_ = front_;
        }
        else
        {
            assert(last_->next == nullptr);

            last_->next = add;
            last_ = add;
            last_->next = nullptr;
        }

        return true;
    }

    void Pop()
    {
        assert(!IsEmpty());

        if (IsEmpty())
        {
            return;
        }

        Node* tmp = front_;
        front_ = front_->next;
        memory::Allocator::Instance().Free(tmp);
		
		full_ = false;
    }

    const NodeData& GetFront() const
    {
        assert(front_ != nullptr);
        return front_->node_data;
    }

   private:
    Node* front_{nullptr};
    Node* last_{nullptr};
    bool full_{false};
};
} // namespace network::tcp::datasegment

#endif // NETWORK_TCP_DATASEGMENTQUEUE_H_
