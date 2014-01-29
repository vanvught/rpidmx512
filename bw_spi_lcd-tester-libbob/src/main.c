#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
//
#include <bcm2835.h>
//
#include <bw.h>
#include <bw_lcd.h>
#include <bw_spi_lcd.h>

unsigned int slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
unsigned int chip_select = 0;

int main(int argc, char **argv) {
	device_info_t lcd_info;

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

	lcd_info.slave_address = slave_address;
	lcd_info.chip_select = chip_select;

	if (bw_spi_lcd_start(&lcd_info)) {
		fprintf(stderr,"Could not start bw_spi_lcd_start library\n");
		exit(1);
	}

	printf("slave address : 0x%x\n", lcd_info.slave_address);
	printf("chip select   : %.1d\n", lcd_info.chip_select);

	printf("bw_spi_lcd_reinit\n");
	bw_spi_lcd_reinit(lcd_info);

	printf("bw_spi_lcd_read_id\n");
	bw_spi_lcd_read_id(lcd_info);

	printf("bw_spi_lcd_cls\n");
	bw_spi_lcd_cls(lcd_info);

	printf("bw_spi_lcd_text_line_1\n");
	bw_spi_lcd_text_line_1(lcd_info, "Raspberry Pi", 12);

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
	    	bw_spi_lcd_text_line_2(lcd_info, buf, 8);
	    	sec = Tm->tm_sec;
	    }
    }

	bw_spi_lcd_end();

	return 0;
}
