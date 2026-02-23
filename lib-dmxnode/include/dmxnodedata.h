/**
 * @file dmxnodedata.h
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

#ifndef DMXNODEDATA_H_
#define DMXNODEDATA_H_

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmxnode.h"

#if defined(GD32)
/**
 * https://www.gd32-dmx.org/memory.html
 */
#include "gd32.h"
#if defined(GD32F450VI) || defined(GD32H7XX)
#define SECTION_LIGHTSET __attribute__((section(".lightset")))
#else
#define SECTION_LIGHTSET
#endif
#else
#define SECTION_LIGHTSET
#endif

namespace dmxnode
{
class Data
{
   public:
    static Data& Get()
    {
        static Data instance SECTION_LIGHTSET;
        return instance;
    }

    static void SetSourceA(uint32_t port_index, const uint8_t* data, uint32_t length) { Get().IMergeSourceA(port_index, data, length, MergeMode::kLtp); }

    static void MergeSourceA(uint32_t port_index, const uint8_t* data, uint32_t length, MergeMode merge_mode) { Get().IMergeSourceA(port_index, data, length, merge_mode); }

    static void SetSourceB(uint32_t port_index, const uint8_t* data, uint32_t length) { Get().IMergeSourceB(port_index, data, length, MergeMode::kLtp); }

    static void MergeSourceB(uint32_t port_index, const uint8_t* data, uint32_t length, MergeMode merge_mode) { Get().IMergeSourceB(port_index, data, length, merge_mode); }

    static void Clear(uint32_t port_index) { Get().IClear(port_index); }

    static void ClearLength(uint32_t port_index) { Get().IClearLength(port_index); }

    static uint32_t GetLength(uint32_t port_index) { return Get().IGetLength(port_index); }

    static const uint8_t* Backup(uint32_t port_index) { return Get().IBackup(port_index); }

    static void Restore(uint32_t port_index, const uint8_t* data) { Get().IRestore(port_index, data); }

   private:
    void IMergeSourceA(uint32_t port_index, const uint8_t* data, uint32_t length, MergeMode merge_mode)
    {
        assert(port_index < kPorts);
        assert(data != nullptr);

        memcpy(output_port_[port_index].source_a.data, data, length);

        output_port_[port_index].length = length;

        if (merge_mode == MergeMode::kHtp)
        {
            for (uint32_t i = 0; i < length; i++)
            {
                const auto kData = std::max(output_port_[port_index].source_a.data[i], output_port_[port_index].source_b.data[i]);
                output_port_[port_index].data[i] = kData;
            }

            return;
        }

        memcpy(output_port_[port_index].data, data, length);
    }

    void IMergeSourceB(uint32_t port_index, const uint8_t* data, uint32_t length, MergeMode merge_mode)
    {
        assert(port_index < kPorts);
        assert(data != nullptr);

        memcpy(output_port_[port_index].source_b.data, data, length);

        output_port_[port_index].length = length;

        if (merge_mode == MergeMode::kHtp)
        {
            for (uint32_t i = 0; i < length; i++)
            {
                const auto kData = std::max(output_port_[port_index].source_a.data[i], output_port_[port_index].source_b.data[i]);
                output_port_[port_index].data[i] = kData;
            }

            return;
        }

        memcpy(output_port_[port_index].data, data, length);
    }

    void IClear(uint32_t port_index)
    {
        assert(port_index < kPorts);

        memset(output_port_[port_index].data, 0, dmxnode::kUniverseSize);
        output_port_[port_index].length = dmxnode::kUniverseSize;
    }

    void IClearLength(uint32_t port_index)
    {
        assert(port_index < kPorts);
        output_port_[port_index].length = 0;
    }

    uint32_t IGetLength(uint32_t port_index) const 
    { 
		assert(port_index < kPorts);
		return output_port_[port_index].length; 
	}

    const uint8_t* IBackup(uint32_t port_index)
    {
        assert(port_index < kPorts);
        return const_cast<const uint8_t*>(output_port_[port_index].data);
    }

    void IRestore(uint32_t port_index, const uint8_t* data)
    {
        assert(port_index < kPorts);
        assert(data != nullptr);

        memcpy(output_port_[port_index].data, data, dmxnode::kUniverseSize);
    }

   private:
#if !defined(DMXNODE_PORTS)
#define DMXNODE_PORTS 0
#endif

#if (DMXNODE_PORTS == 0)
    static constexpr auto kPorts = 1; // ISO C++ forbids zero-size array
#else
    static constexpr auto kPorts = DMXNODE_PORTS;
#endif

    struct Source
    {
        uint8_t data[dmxnode::kUniverseSize] __attribute__((aligned(4)));
    };

    struct OutputPort
    {
        Source source_a;
        Source source_b;
        uint8_t data[dmxnode::kUniverseSize] __attribute__((aligned(4)));
        uint32_t length;
    };

    OutputPort output_port_[kPorts];
};
} // namespace dmxnode

#endif  // DMXNODEDATA_H_
