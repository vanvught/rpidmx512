/**
 * @file ubootheader.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef UBOOTHEADER_H_
#define UBOOTHEADER_H_

#include <cstdint>

#define LZ4F_MAGIC 0x184D2204 /* LZ4 Magic Number		*/
#define IH_MAGIC 0x27051956   /* Image Magic Number	*/
#define IH_NMLEN 32           /* Image Name Length	*/
#define IH_OS_U_BOOT 17
#define IH_ARCH_ARM 2
#define IH_TYPE_STANDALONE 1

struct TImageHeader
{
    uint32_t ih_magic;         /* Image Header Magic Number	*/
    uint32_t ih_hcrc;          /* Image Header CRC Checksum	*/
    uint32_t ih_time;          /* Image Creation Timestamp	*/
    uint32_t ih_size;          /* Image Data Size		*/
    uint32_t ih_load;          /* Data	 Load  Address		*/
    uint32_t ih_ep;            /* Entry Point Address		*/
    uint32_t ih_dcrc;          /* Image Data CRC Checksum	*/
    uint8_t ih_os;             /* Operating System		*/
    uint8_t ih_arch;           /* CPU architecture		*/
    uint8_t ih_type;           /* Image Type			*/
    uint8_t ih_comp;           /* Compression Type		*/
    uint8_t ih_name[IH_NMLEN]; /* Image Name		*/
};

enum TImageHeaderCompression
{
    IH_COMP_NONE = 0, /*  No	 Compression Used	*/
    IH_COMP_GZIP      /* gzip	 Compression Used	*/
};

class UBootHeader
{
   public:
    explicit UBootHeader(const uint8_t* header);
    ~UBootHeader() { is_valid_ = false; }

    bool IsValid() const { return is_valid_; }

    bool IsCompressed() const { return is_compressed_; }

    void Dump();

   private:
    const uint8_t* header_;
    bool is_valid_{false};
    bool is_compressed_{false};
};

#endif  // UBOOTHEADER_H_
