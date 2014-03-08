/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <time.h>
//
#include <bw_spi_lcd.h>

unsigned int slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
unsigned int chip_select = 0;

int main(int argc, char **argv) {
	device_info_t device_info;

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

	if (bw_spi_lcd_start(&device_info)) {
		fprintf(stderr,"Could not start bw_spi_lcd_start\n");
		exit(EXIT_FAILURE);
	}

	printf("slave address : 0x%x\n", device_info.slave_address);
	printf("chip select   : %.1d\n", device_info.chip_select);

	printf("bw_spi_lcd_reinit\n");
	bw_spi_lcd_reinit(&device_info);

	printf("bw_spi_lcd_read_id\n");
	bw_spi_lcd_read_id(&device_info);

	printf("bw_spi_lcd_cls\n");
	bw_spi_lcd_cls(&device_info);

	printf("bw_spi_lcd_text_line_1\n");
	bw_spi_lcd_text_line_1(&device_info, "Raspberry Pi", 12);

	printf("bw_spi_lcd_text_line_2\n");

	time_t ltime;
	struct tm *Tm;
	int sec = -1;
	char buf[BW_LCD_MAX_CHARACTERS];

	for (;;) {
	    ltime = time(NULL);
	    Tm = localtime(&ltime);
	    if (sec != Tm->tm_sec) {
	    	sprintf(buf, "%.2d:%.2d:%.2d", Tm->tm_hour, Tm->tm_min, Tm->tm_sec);
	    	bw_spi_lcd_text_line_2(&device_info, buf, 8);
	    	sec = Tm->tm_sec;
	    }
    }

	bw_spi_lcd_end();

	return 0;
}
