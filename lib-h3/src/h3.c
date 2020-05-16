/**
 * @file h3.c
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "h3.h"

#define MCTL_COM_BASE	0x01c62000
#define MC_CR			(MCTL_COM_BASE + 0x0)

inline static uint32_t _mctl_read(uint32_t reg) {
	return *(volatile uint32_t *) (reg);
}

// https://github.com/allwinner-zh/bootloader/blob/e5ceeca211883d9f6b25f84e0e3c5fe2afaaf350/basic_loader/bsp/bsp_for_a33/init_dram/mctl_hal.c#L1107
uint32_t h3_get_dram_size(void) {
	uint32_t value;
	uint32_t dram_size;
	uint32_t temp;

	value = _mctl_read(MC_CR);

	temp = (value >> 8) & 0xf;	//page size code
	dram_size = (temp - 6);		//(1<<dram_size) * 512Bytes

	temp = (value >> 4) & 0xf;	//row width code
	dram_size += (temp + 1);	//(1<<dram_size) * 512Bytes

	temp = (value >> 2) & 0x3;	//bank number code
	dram_size += (temp + 2);	//(1<<dram_size) * 512Bytes

	temp = value & 0x3;			//rank number code
	dram_size += temp;			//(1<<dram_size) * 512Bytes

	dram_size = dram_size - 11;	//(1<<dram_size)MBytes

	return (1U << dram_size);
}

// https://github.com/linux-sunxi/sunxi-tools/blob/master/uart0-helloworld-sdboot.c#L458
h3_boot_device_t h3_get_boot_device(void) {
	uint32_t *spl_signature = (void *) 0x4;

	/* Check the eGON.BT0 magic in the SPL header */
	if (spl_signature[0] != 0x4E4F4765 || spl_signature[1] != 0x3054422E)
		return H3_BOOT_DEVICE_FEL;

	const uint32_t boot_dev = spl_signature[9] & 0xFF; /* offset into SPL = 0x28 */

	if (boot_dev == 0) {
		return H3_BOOT_DEVICE_MMC0;
	}

	if (boot_dev == 3) {
		return H3_BOOT_DEVICE_SPI;
	}

	return H3_BOOT_DEVICE_UNK;
}
