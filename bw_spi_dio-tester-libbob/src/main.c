#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//
#include <bw.h>
#include <bw_dio.h>
#include <bw_spi_dio.h>

unsigned int slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;
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

	if (bw_spi_dio_start(&lcd_info)) {
		fprintf(stderr,"Could not start bw_spi_dio_start\n");
		exit(1);
	}

	printf("slave address : 0x%x\n", lcd_info.slave_address);
	printf("chip select   : %.1d\n", lcd_info.chip_select);

	printf("bw_spi_dio_read_id\n");
	bw_spi_dio_read_id(lcd_info);

	printf("bw_spi_dio_fsel_mask\n");
	bw_spi_dio_fsel_mask(lcd_info, 0x7F);

	printf("bw_spi_dio_output ...\n");

	for(;;) {
		bw_spi_dio_output(lcd_info, BW_DIO_PIN_IO0 | BW_DIO_PIN_IO2 | BW_DIO_PIN_IO4 | BW_DIO_PIN_IO6);
		sleep(1);
		bw_spi_dio_output(lcd_info, BW_DIO_PIN_IO1 | BW_DIO_PIN_IO3 | BW_DIO_PIN_IO5);
		sleep(1);
	}

	bw_spi_dio_end();

	return 0;
}
