/**
 * @file oscblob.h
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCBLOB_H_
#define OSCBLOB_H_

#include <cstdint>

/*
 * OSC-blob
 * An int32 size count, followed by that many 8-bit bytes of arbitrary binary data,
 * followed by 0-3 additional zero bytes to make the total number of bits a multiple of 32.
 */

class OSCBlob
{
   public:
    OSCBlob(const uint8_t* data, uint32_t size) : data_(data), size_(size) {}

    uint32_t GetDataSize() const { return size_; }

    const uint8_t* GetDataPtr() const { return data_; }

    uint32_t GetSize()
    {
        const uint32_t kBlobSize = sizeof(int32_t) + size_;
        return (4 * ((kBlobSize + 3) / 4));
    }

    uint8_t GetByte(uint32_t index)
    {
        if (index < size_)
        {
            return data_[index];
        }
        return 0;
    }

   private:
    const uint8_t* data_;
    uint32_t size_;
};

#endif  // OSCBLOB_H_
