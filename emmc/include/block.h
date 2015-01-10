/**
 * @file block.h
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
 * Based on
 * https://github.com/jncronin/rpi-boot/blob/master/block.h
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

#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

struct block_device {
	int supports_multiple_block_read;
	int supports_multiple_block_write;

	int (*read)(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_num);
	int (*write)(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_num);

	size_t block_size;
	size_t num_blocks;
};

size_t block_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block);
size_t block_write(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block);

#endif

