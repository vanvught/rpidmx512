/**
 * @file new.cpp
 *
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

#include <cstdlib>
#include <cstddef>

/*
 * Pre-C++14 and onward:
 * These are the global operator new overloads for single objects (new) and arrays (new[]).
 * They allocate memory from the heap using malloc.
 * This implementation does NOT throw std::bad_alloc if malloc returns nullptr,
 * which is common in embedded or standalone environments.
 */

void *operator new(std::size_t size) {
	return malloc(size);
}

void *operator new[](std::size_t size) {
	return malloc(size);
}

/*
 * Introduced in C++17:
 * Placement new overload allows constructing objects at a specific memory location.
 * This overload simply returns the provided memory address (ptr).
 * Placement new does not allocate memory.
 */

void *operator new([[maybe_unused]] std::size_t, void *ptr) noexcept {
    return ptr;
}
