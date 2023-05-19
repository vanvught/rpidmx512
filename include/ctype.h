/**
 * @file ctype.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef CTYPE_H_
#define CTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

inline int isdigit(int c) {
	return (c >= (int) '0' && c <= (int) '9') ? 1 : 0;
}

inline int isxdigit(int c) {
	return ((isdigit(c) != 0) || (((unsigned) c | 32) - (int) 'a' < 6)) ? 1 : 0;
}

inline int isprint(int c) {
	return ((c >= (int) ' ' && c <= (int) '~')) ? 1 : 0;
}

inline int isupper(int c) {
	return (c >= (int) 'A' && c <= (int) 'Z') ? 1 : 0;
}

inline int islower(int c) {
	return (c >= (int) 'a' && c <= (int) 'z') ? 1 : 0;
}

inline int isalpha(int c) {
	return ((isupper(c) != 0) || (islower(c) != 0)) ? 1 : 0;
}

inline int tolower(int c) {
	return ((isupper(c) != 0) ? (c + 32) : c);
}

inline int toupper(int c) {
	return ((islower(c) != 0) ? (c - 32) : c);
}

inline int isspace(int c) {
	return (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ' ? 1 : 0);
}

#ifdef __cplusplus
}
#endif

#endif /* CTYPE_H_ */
