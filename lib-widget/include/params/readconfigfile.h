/**
 * @file readconfigfile.h
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

#ifndef PARAMS_READCONFIGFILE_H_
#define PARAMS_READCONFIGFILE_H_

#include <cstdint>

typedef void (*CallbackFunctionPtr)(void*, const char*);

class ReadConfigFile
{
   public:
    ReadConfigFile(CallbackFunctionPtr callback, void* p);
    ~ReadConfigFile();

#if !defined(DISABLE_FS)
    bool Read(const char* filename);
#endif
    void Read(const char* buffer, uint32_t length);

   private:
    CallbackFunctionPtr callback_;
    void* m_p;
};

#endif // PARAMS_READCONFIGFILE_H_
