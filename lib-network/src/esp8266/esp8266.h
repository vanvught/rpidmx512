/**
 * @file esp8266.h
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ESP8266_H_
#define ESP8266_H_

#include <cstdint>

void esp8266_init();
bool esp8266_detect();

void esp8266_write_4bits(const uint8_t);
void esp8266_write_byte(const uint8_t);
void esp8266_write_halfword(const uint16_t);
void esp8266_write_word(const uint32_t);
void esp8266_write_bytes(const uint8_t *, const uint32_t);
void esp8266_write_str(const char *);

uint8_t esp8266_read_byte();
void esp8266_read_bytes(const uint8_t *, const uint32_t);
uint16_t esp8266_read_halfword();
uint32_t esp8266_read_word();
void esp8266_read_str(char *, uint32_t *);

#endif /* ESP8266_H_ */
