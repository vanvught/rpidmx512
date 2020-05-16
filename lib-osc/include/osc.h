/**
 * @file osc.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSC_H_
#define OSC_H_

#include <stdint.h>

#define OSC_MAX_MSG_SIZE 1024 //TODO OSC_MAX_MSG_SIZE

#define OSC_DEFAULT_INCOMING_PORT	8000
#define OSC_DEFAULT_OUTGOING_PORT	9000

typedef struct {
	/** The number of seconds since Jan 1st 1900 in the UTC timezone. */
	uint32_t sec;
	/** The fractions of a second offset from above, expressed as 1/2^32nds of a second */
	uint32_t frac;
} osc_timetag;

typedef union {
	/** 32 bit signed integer. */
    int32_t    i;
	/** 32 bit signed integer. */
    int32_t    i32;
	/** 64 bit signed integer. */
    int64_t    h;
	/** 64 bit signed integer. */
    int64_t    i64;
	/** 32 bit IEEE-754 float. */
    float      f;
	/** 32 bit IEEE-754 float. */
    float      f32;
	/** 64 bit IEEE-754 double. */
    double     d;
	/** 64 bit IEEE-754 double. */
    double     f64;
	/** Standard C, NULL terminated string. */
    char       s;
	/** Standard C, NULL terminated, string. Used in systems which distinguish strings and symbols. */
    char       S;
	/** Standard C, 8 bit, char. */
    unsigned char c;
	/** A 4 byte MIDI packet. */
    uint8_t    m[4];
	/** OSC TimeTag value. */
    osc_timetag t;
} osc_arg;

struct OscType {
	/* basic OSC types */
	/* 32 bit signed integer. */
	static constexpr char INT32 = 'i';
	/* 32 bit IEEE-754 float. */
	static constexpr char FLOAT = 'f';
	/* Standard C, NULL terminated string. */
	static constexpr char STRING = 's';
	/* OSC binary blob type. */
	static constexpr char BLOB = 'b';
	/* extended OSC types */
	/* 64 bit signed integer. */
	static constexpr char INT64 = 'h';
	/* OSC TimeTag type. */
	static constexpr char TIMETAG = 't';
	/* 64 bit IEEE-754 double. */
	static constexpr char DOUBLE = 'd';
	/* Standard C, NULL terminated, string. Used in systems which distinguish strings and symbols. */
	static constexpr char SYMBOL = 'S';
	/* Standard C, 8 bit, char variable. */
	static constexpr char CHAR = 'c';
	/* A 4 byte MIDI packet. */
	static constexpr char MIDI = 'm';
	/* Symbol representing the value True. */
	static constexpr char TRUE = 'T';
	/* Symbol representing the value False. */
	static constexpr char FALSE = 'F';
	/* Symbol representing the value Nil. */
	static constexpr char NIL = 'N';
	/* Symbol representing the value Infinitum. */
	static constexpr char INFINITUM = 'I';
	/* */
	static constexpr char UNKNOWN = '\0';
};

#include "oscstring.h"

extern "C" {
int lo_pattern_match(const char *, const char *);
}

class OSC {
public:
	 static char *GetPath(void *pPath, int nSize) {
		return (OSCString::Validate(pPath, nSize) >= 4) ? reinterpret_cast<char *>(pPath) : 0;
	}

	 static bool isMatch(const char *str, const char *p) {
		return lo_pattern_match(str, p) == 0 ? false : true;
	}
};

#endif /* OSC_H_ */
