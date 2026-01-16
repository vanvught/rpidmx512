/**
 * @file malloc.cpp
 *
 */
/* This code is inspired by:
 *
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2016  R. Stange <rsta2@o2online.de>
 * https://github.com/rsta2/circle
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#ifdef DEBUG_HEAP
#undef NDEBUG
#endif

#include <cstddef>
#include <cstdint>
#ifdef DEBUG_HEAP
#include <cstdio>
#endif
#include <cassert>

namespace console
{
void Error(const char*);
}

void debug_heap();

struct BlockHeader
{
    unsigned int magic;
    unsigned int size;
    struct BlockHeader* next;
    unsigned char data;
} __attribute__((packed));

struct BlockBucket
{
    unsigned int size;
#ifdef DEBUG_HEAP
    unsigned int count;
    unsigned int max_count;
#endif
    struct BlockHeader* free_list;
};

extern unsigned char heap_low; /* Defined by the linker */
extern unsigned char heap_top; /* Defined by the linker */

static unsigned char* next_block = &heap_low;
static unsigned char* block_limit = &heap_top;

static constexpr unsigned int kBlockMagic = 0x424C4D43;

#if defined(H3)
#include "h3/malloc.h"
#elif defined(GD32)
#include "gd32/malloc.h"
#else
#include "rpi/malloc.h"
#endif

static size_t GetAllocated(void* p)
{
    if (p == nullptr)
    {
        return 0;
    }

    auto* block_header = reinterpret_cast<struct BlockHeader*>(reinterpret_cast<uintptr_t>(p) - offsetof(BlockHeader, data));

    assert(block_header->magic == kBlockMagic);

    if (block_header->magic != kBlockMagic)
    {
        return 0;
    }

    return block_header->size;
}

extern "C"
{
    void* malloc(size_t size) //NOLINT
    {
        struct BlockBucket* bucket;

        if (size == 0)
        {
            return nullptr;
        }

        for (bucket = s_block_bucket; bucket->size > 0; bucket++)
        {
            if (size <= bucket->size)
            {
                size = bucket->size;
#ifdef DEBUG_HEAP
                if (++bucket->count > bucket->max_count)
                {
                    bucket->max_count = bucket->count;
                }
#endif
                break;
            }
        }

        struct BlockHeader* header;

        if (bucket->size > 0 && (header = bucket->free_list) != nullptr)
        {
            assert(header->magic == kBlockMagic);
            bucket->free_list = header->next;
        }
        else
        {
            header = reinterpret_cast<struct BlockHeader*>(next_block);

            const auto kT1 = sizeof(struct BlockHeader) + size;
            const auto kT2 = (kT1 + 15) & static_cast<size_t>(~15);

            auto* next = next_block + kT2;

            assert((reinterpret_cast<uintptr_t>(header) & 3U) == 0);
            assert((reinterpret_cast<uintptr_t>(next) & 3U) == 0);

            if (next > block_limit)
            {
                console::Error("malloc: out of memory");
#ifdef DEBUG_HEAP
                debug_heap();
#endif
                return nullptr;
            }

            next_block = next;

            header->magic = kBlockMagic;
            header->size = size;
        }

        header->next = nullptr;
#ifdef DEBUG_HEAP
        printf("malloc(%u): pBlockHeader=%p, size=%u, data=%p\n", size, header, header->size, reinterpret_cast<void*>(&header->data));
#endif

        assert((reinterpret_cast<uintptr_t>(&header->data) & 3U) == 0);
        return reinterpret_cast<void*>(&header->data);
    }

    void free(void* p) //NOLINT
    {
        struct BlockBucket* bucket;

        if (p == nullptr)
        {
            return;
        }

        auto* header = reinterpret_cast<struct BlockHeader*>(reinterpret_cast<uintptr_t>(p) - offsetof(BlockHeader, data));

#ifdef DEBUG_HEAP
        printf("free: header= %p, p=%p, size=%u\n", header, p, header->size);
#endif

        assert(header->magic == kBlockMagic);
        if (header->magic != kBlockMagic)
        {
            return;
        }

        for (bucket = s_block_bucket; bucket->size > 0; bucket++)
        {
            if (header->size == bucket->size)
            {
                header->next = bucket->free_list;
                bucket->free_list = header;
#ifdef DEBUG_HEAP
                if (bucket->count > 0)
                {
                    bucket->count--;
                }
#endif
                break;
            }
        }
    }

    void* calloc(size_t n, size_t size)//NOLINT
    {
        if ((n == 0) || (size == 0))
        {
            return nullptr;
        }

        auto total = n * size;
        auto* p = malloc(total);

        if (p == nullptr)
        {
            return nullptr;
        }

        assert((reinterpret_cast<uintptr_t>(p) & 3U) == 0);

        auto* dst32 = reinterpret_cast<uint32_t*>(p);

        while (total >= 4)
        {
            *dst32++ = 0;
            total -= 4;
        }

        auto* dst8 = reinterpret_cast<uint8_t*>(dst32);

        while (total--)
        {
            *dst8++ = 0;
        }

        assert((reinterpret_cast<uintptr_t>(dst8) - reinterpret_cast<uintptr_t>(p)) == (n * size));

        return p;
    }

    void* realloc(void* ptr, size_t newsize) //NOLINT
    {
        if (ptr == nullptr)
        {
            auto* newblk = malloc(newsize);
            return newblk;
        }

        if (newsize == 0)
        {
            free(ptr);
            return nullptr;
        }

        auto current_size = GetAllocated(ptr);

        if (current_size >= newsize)
        {
            return ptr;
        }

        void* newblk = malloc(newsize);

        if (newblk != nullptr)
        {
            assert((reinterpret_cast<uintptr_t>(newblk) & 3U) == 0);
            assert((reinterpret_cast<uintptr_t>(ptr) & 3U) == 0);

            auto* src32 = reinterpret_cast<const uint32_t*>(ptr);
            auto* dst32 = reinterpret_cast<uint32_t*>(newblk);

            auto count = newsize;

            while (count >= 4)
            {
                *dst32++ = *src32++;
                count -= 4;
            }

            auto* src8 = reinterpret_cast<const uint8_t*>(src32);
            auto* dst8 = reinterpret_cast<uint8_t*>(dst32);

            while (count--)
            {
                *dst8++ = *src8++;
            }

            assert((reinterpret_cast<uintptr_t>(dst8) - reinterpret_cast<uintptr_t>(newblk)) == newsize);

            free(ptr);
        }

        return newblk;
    }
}

void debug_heap()
{
#ifdef DEBUG_HEAP
    printf("next_block = %p\n", next_block);

    struct BlockBucket* bucket;

    for (bucket = s_block_bucket; bucket->size > 0; bucket++)
    {
        struct BlockHeader* free_list = bucket->free_list;
        printf("malloc(%d): %d blocks (max %d), FreeList %p (next %p)\n", bucket->size, bucket->count, bucket->max_count, free_list, free_list->next);
        struct BlockHeader* block_header;

        auto freelist_count = bucket->max_count - bucket->count;

        if ((block_header = bucket->free_list) != nullptr)
        {
            while (freelist_count-- > 0)
            {
                printf("\t %p:%p size %d (next %p)\n", block_header, reinterpret_cast<void*>(&block_header->data), block_header->size, block_header->next);
                block_header = block_header->next;
            }
        }
    }
#endif
}
