/**
 * @file perror.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <stddef.h>
#include <errno.h>

namespace console
{
void Error(const char*);
int Putc(int);
int Puts(const char*);
} // namespace console

/*
errno -l | cut -f3- -d ' ' | sort -V -u | awk '$0="\""$0"\","'
*/

const char* const kSysErrlist[] = {"OK",
                                   "Operation not permitted",
                                   "No such file or directory",
                                   "No such process",
                                   "Interrupted system call",
                                   "Input/output error",
                                   "No such device or address",
                                   "Argument list too long",
                                   "Exec format error",
                                   "Bad file descriptor",
                                   "No child processes",
                                   "Resource temporarily unavailable",
                                   "Cannot allocate memory",
                                   "Permission denied",
                                   "Bad address",
                                   "Block device required",
                                   "Device or resource busy",
                                   "File exists",
                                   "Invalid cross-device link",
                                   "No such device",
                                   "Not a directory",
                                   "Is a directory",
                                   "Invalid argument",
                                   "Too many open files in system",
                                   "Too many open files",
                                   "Inappropriate ioctl for device",
                                   "Text file busy",
                                   "File too large",
                                   "No space left on device",
                                   "Illegal seek",
                                   "Read-only file system",
                                   "Too many links",
                                   "Broken pipe",
                                   "Numerical argument out of domain",
                                   "Numerical result out of range",
                                   "Resource deadlock avoided",
                                   "File name too long",
                                   "No locks available",
                                   "Function not implemented",
                                   "Directory not empty",
                                   "Bad message"};

extern "C"
{
    char* strerror(int errnum) // NOLINT
    {
        if (errnum <= ELAST)
        {
            return const_cast<char*>(kSysErrlist[errnum]);
        }

        return const_cast<char*>(kSysErrlist[EBADMSG]);
    }

    void perror(const char* s) // NOLINT
    {
        const char* ptr = nullptr;

        if (errno >= 0 && errno < ELAST)
        {
            ptr = kSysErrlist[errno];
        }
        else
        {
            ptr = kSysErrlist[EBADMSG];
        }

        if (s && *s)
        {
            console::Error(s);
            console::Puts(": ");
        }

        console::Error(ptr);
        console::Putc('\n');
    }
}
