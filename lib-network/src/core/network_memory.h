/**
 * @file network_memory.h
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

#ifndef NETWORK_MEMORY_H_
#define NETWORK_MEMORY_H_

#include <cstdint>
#include <cstring>
#include <cassert>

namespace console
{
void Error(const char*);
}

namespace network::memory
{
inline constexpr uint32_t kBlocks =
#if !defined(CONFIG_NETWORK_MEMORY_BLOCKS)
    8;
#else
    CONFIG_NETWORK_MEMORY_BLOCKS;
#endif

static_assert(kBlocks >= 1);
static_assert(kBlocks <= 32);

inline constexpr uint32_t kBlockSize =
#if !defined(CONFIG_NETWORK_MEMORY_BLOCKSIZE)
    1460;
#else
    CONFIG_NETWORK_MEMORY_BLOCKSIZE;
#endif

static_assert((kBlockSize % 4) == 0);

extern uint8_t pool[kBlocks][kBlockSize] __attribute__((aligned(4)));

class Allocator
{
   public:
    static Allocator& Instance()
    {
        static Allocator instance;
        static bool inited = false;

        if (!inited) [[unlikely]] // one-time init
        {
            instance.Init();
            inited = true;
        }

        return instance;
    }

    void Init()
    {
        free_mask_ = kAllMask;
        std::memset(size_, 0, sizeof(size_));
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    bool IsEmpty() const { return free_mask_ == kAllMask; }
    bool IsFull() const { return free_mask_ == 0; }

    uint8_t* Allocate()
    {
        if (IsFull())
        {
            console::Error("Full!");
            return nullptr;
        }

        const uint32_t kIndex = static_cast<uint32_t>(__builtin_ctz(free_mask_));
        free_mask_ &= ~(1u << kIndex);

        Status();

        return pool[kIndex];
    }

    uint16_t Allocate(const uint8_t* data, uint16_t size)
    {
        assert(data != nullptr);
        assert(size > 0);
        assert(size <= kBlockSize);

        if (IsFull())
        {
            console::Error("Full!");
            return UINT16_MAX;
        }

        const uint32_t kIndex = static_cast<uint32_t>(__builtin_ctz(free_mask_));
        free_mask_ &= ~(1u << kIndex);

        size_[kIndex] = size;
        memcpy(pool[kIndex], data, size);

        Status();

        return static_cast<uint16_t>(kIndex);
    }

    void Free(void* pointer)
    {
        assert(pointer != nullptr);

        for (uint32_t index = 0; index < kBlocks; ++index)
        {
            if (static_cast<void*>(pool[index]) == pointer)
            {
                return Free(static_cast<uint16_t>(index));
            }
        }

        assert(false); // pointer not from pool
    }

    void Free(uint16_t index)
    {
        if (index == UINT16_MAX) return;

        assert(index < kBlocks);

        const uint32_t kBit = (1U << index);
        assert((free_mask_ & kBit) == 0); // detect double free
        free_mask_ |= kBit;

        size_[index] = 0;

        Status();
    }

    uint8_t* Get(uint16_t index, uint32_t& size)
    {
        assert(index < kBlocks);
        assert(size_[index] != 0);

        size = size_[index];
        return pool[index];
    }

    void Status() const
    {
#if defined DEBUG_NETWORK_MEMORY
        const uint32_t kUsedMask = (~free_mask_) & kAllMask;
        printf("free_mask=0x%08x used_mask=0x%08x free=%u used=%u\n", free_mask_, kUsedMask, __builtin_popcount(free_mask_), __builtin_popcount(kUsedMask));
        printf("IsEmpty=%c IsFull=%c\n", IsEmpty() ? 'Y' : 'N', IsFull() ? 'Y' : 'N');
#endif
    }

   private:
    Allocator() = default;
    static constexpr uint32_t kAllMask = (kBlocks == 32) ? UINT32_MAX : ((1U << kBlocks) - 1U);
    uint32_t free_mask_{0};
    uint16_t size_[kBlocks]{0};
};
} // namespace network::memory

#endif // NETWORK_MEMORY_H_
