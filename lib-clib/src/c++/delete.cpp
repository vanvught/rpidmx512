/**
 * @file delete.cpp
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@info@gd32-dmx.org
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

#include <cstdlib>
#include <cstddef>

/*
 * Pre-C++14 and onward.
 * These are the global operator delete overloads for single objects (delete) and arrays (delete[]).
 */

void operator delete(void *p) {
	free(p);
}

void operator delete[](void *p) {
	free(p);
}

/*
 * Introduced in C++14.
 * These overloads include the size of the object being deleted as an additional parameter (std::size_t size).
 */

void operator delete(void *p, [[maybe_unused]] std::size_t size) noexcept {
	free(p);
}

void operator delete[](void *p, [[maybe_unused]]std::size_t size) noexcept {
	free(p);
}

/*
 * Introduced in C++17,
 * This overload is rarely needed in practice but ensures completeness for cases where delete might be called generically.
 */

void operator delete([[maybe_unused]] void *, [[maybe_unused]] void *) noexcept {
    // No-op
}

