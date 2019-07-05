/**
 * @file si5351a.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdbool.h>
#if defined (__linux__)
 #include <stdio.h>
 #include <stdlib.h>
#endif

#include "si5351a.h"

#include "bob.h"

#if defined (__linux__)
 #define DEFAULT_CSV_FILE_NAME	"Si5351A-RevB-Registers.txt"
 #define DEFAULT_H_FILE_NAME	"Si5351A-RevB-Registers.h"

 #define MAXCHAR 96
#endif

si5351a_revb_register_t const si5351a_revb_registers[SI5351A_REVB_REG_CONFIG_NUM_REGS] =
{
	{ 0x0002, 0x53 },
	{ 0x0003, 0x00 },
	{ 0x0007, 0x00 },
	{ 0x000F, 0x00 },
	{ 0x0010, 0x0F },
	{ 0x0011, 0x0F },
	{ 0x0012, 0x0F },
	{ 0x0013, 0x8C },
	{ 0x0014, 0x8C },
	{ 0x0015, 0x8C },
	{ 0x0016, 0x8C },
	{ 0x0017, 0x8C },
	{ 0x001A, 0x00 },
	{ 0x001B, 0x7D },
	{ 0x001C, 0x00 },
	{ 0x001D, 0x0F },
	{ 0x001E, 0xFB },
	{ 0x001F, 0x00 },
	{ 0x0020, 0x00 },
	{ 0x0021, 0x71 },
	{ 0x002A, 0x00 },
	{ 0x002B, 0x01 },
	{ 0x002C, 0x12 },
	{ 0x002D, 0x30 },
	{ 0x002E, 0x00 },
	{ 0x002F, 0x00 },
	{ 0x0030, 0x00 },
	{ 0x0031, 0x00 },
	{ 0x0032, 0x00 },
	{ 0x0033, 0x01 },
	{ 0x0034, 0x02 },
	{ 0x0035, 0x30 },
	{ 0x0036, 0x00 },
	{ 0x0037, 0x00 },
	{ 0x0038, 0x00 },
	{ 0x0039, 0x00 },
	{ 0x003A, 0x00 },
	{ 0x003B, 0x02 },
	{ 0x003C, 0x00 },
	{ 0x003D, 0x44 },
	{ 0x003E, 0x40 },
	{ 0x003F, 0x00 },
	{ 0x0040, 0x00 },
	{ 0x0041, 0x00 },
	{ 0x005A, 0x00 },
	{ 0x005B, 0x00 },
	{ 0x0095, 0x00 },
	{ 0x0096, 0x00 },
	{ 0x0097, 0x00 },
	{ 0x0098, 0x00 },
	{ 0x0099, 0x00 },
	{ 0x009A, 0x00 },
	{ 0x009B, 0x00 },
	{ 0x00A2, 0x00 },
	{ 0x00A3, 0x00 },
	{ 0x00A4, 0x00 },
	{ 0x00B7, 0xD2 },
};

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

static void pre_regs(void) {
	uint32_t i;

	/*
	 * Disable Outputs. Set CLKx_DIS high; Reg. 3 = 0xFF
	 */
	i2c_write_reg_uint8(3, 0xFF);

	/*
	 * Powerdown all output drivers Reg. 16, 17, 18, 19, 20, 21, 22, 23 = 0x80
	 */
	for (i = 16 ; i <= 23; i++) {
		i2c_write_reg_uint8(i, 0x80);
	}
}

static void post_regs(void) {
	/**
	 * Apply PLLA and PLLB soft reset Reg. 177 = 0xAC
	 */
	i2c_write_reg_uint8(177, 0xAC);

	/*
	 * Enable outputs
	 */
	i2c_write_reg_uint8(3, 0x00);
}

bool si5351a_start(device_info_t *device_info) {
	i2c_begin();

	if (device_info->slave_address == 0) {
		device_info->slave_address = SI5351A_I2C_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == 0) {
		device_info->fast_mode = true;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	return true;
}

void si5351a_dump(device_info_t *device_info) {
	i2c_setup(device_info);
}

bool si5351a_clock_builder(const device_info_t *device_info) {
	uint32_t i;

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	pre_regs();

	for (i = 0; i < sizeof(si5351a_revb_registers) / sizeof(si5351a_revb_registers[0]); i++) {
		i2c_write_reg_uint8(si5351a_revb_registers[i].address, si5351a_revb_registers[i].value);
	}

	post_regs();

	return true;
}

#if defined (__linux__)
bool si5351a_csv(const device_info_t *device_info, const char *file_name) {
	FILE *fp;
	char str[MAXCHAR];
	int reg, value;

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	if (file_name == NULL) {
		fp = fopen(DEFAULT_CSV_FILE_NAME, "r");
	} else {
		fp = fopen(file_name, "r");
	}

	if (fp == NULL) {
		return false;
	}

	pre_regs();

	while (fgets(str, MAXCHAR, fp) != NULL) {
		if (sscanf(str, "%x,%x", &reg, &value) == 2) {
			printf("%.4X:%.2X\n", reg, value);
			i2c_write_reg_uint8(reg, value);
		} else {
		    fprintf(stderr, "No matching characters\n");
		}
	}

	post_regs();

	fclose(fp);

	return true;
}
#endif
