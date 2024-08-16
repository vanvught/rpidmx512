/**
 * @file assert.c
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <stdio.h>

/*
 * Description is taken from newlib https://sourceware.org/newlib/
 *
 * DESCRIPTION
	Use this macro to embed debugging diagnostic statements in
	your programs.  The argument <[expression]> should be an
	expression which evaluates to true (nonzero) when your program
	is working as you intended.
	When <[expression]> evaluates to false (zero), <<assert>>
	calls <<abort>>, after first printing a message showing what
	failed and where:

. Assertion failed: <[expression]>, file <[filename]>, line <[lineno]>, function: <[func]>

	If the name of the current function is not known (for example,
	when using a C89 compiler that does not understand __func__),
	the function location is omitted.
	The macro is defined to permit you to turn off all uses of
	<<assert>> at compile time by defining <<NDEBUG>> as a
	preprocessor variable.   If you do this, the <<assert>> macro
	expands to

. (void(0))
 */

void __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
	printf("assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr, file, line, func ? ", function: " : "", func ? func : "");

	for (;;)
		;
}
