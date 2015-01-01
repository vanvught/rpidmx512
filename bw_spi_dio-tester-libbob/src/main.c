/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <stdlib.h>
#include <unistd.h>
#include "bw_spi.h"
#include "bw_spi_dio.h"

int main(int argc, char **argv) {
	device_info_t device_info;

	unsigned int slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;
	unsigned int chip_select = 0;

	while ((argc > 1) && (argv[1][0] == '-')) {
		switch (argv[1][1]) {
		case 'a':
			sscanf (&argv[1][2], "%x", &slave_address);
			break;
		case 'c':
			sscanf (&argv[1][2], "%d", &chip_select);
			break;
		default:
			fprintf(stderr, "Warning: Unsupported argument \"%s\"!\n", argv[1]);
			exit(1);
		}
		++argv;
		--argc;
	}

	device_info.slave_address = slave_address;
	device_info.chip_select = chip_select;

	if (bw_spi_dio_start(&device_info)) {
		fprintf(stderr,"Could not start bw_spi_dio_start\n");
		exit(EXIT_FAILURE);
	}

	printf("slave address : 0x%x\n", device_info.slave_address);
	printf("chip select   : %.1d\n", device_info.chip_select);

	printf("bw_spi_read_id\n");
	bw_spi_read_id(&device_info);

	printf("bw_spi_dio_fsel_mask\n");
	bw_spi_dio_fsel_mask(&device_info, 0x7F);

	printf("bw_spi_dio_output ...\n");

	for(;;) {
		bw_spi_dio_output(&device_info, BW_DIO_PIN_IO0 | BW_DIO_PIN_IO2 | BW_DIO_PIN_IO4 | BW_DIO_PIN_IO6);
		sleep(1);
		bw_spi_dio_output(&device_info, BW_DIO_PIN_IO1 | BW_DIO_PIN_IO3 | BW_DIO_PIN_IO5);
		sleep(1);
	}

	bw_spi_dio_end();

	return 0;
}
