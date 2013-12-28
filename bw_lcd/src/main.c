#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <bw_lcd.h>

int main(int argc, char **argv) {

	printf("lcd_start\n");
	if (lcd_start(0x00) != BW_LCD_OK) {
		printf("Cannot start LCD\n");
		exit(EXIT_FAILURE);
	}

	printf("lcd_read_id\n");
	lcd_read_id();

	printf("lcd_reinit\n");
	lcd_reinit();

	printf("lcd_cls\n");
	lcd_cls();

	printf("lcd_text_line_1\n");
	lcd_text_line_1("Raspberry Pi", 12);

	printf("lcd_text_line_2\n");
	time_t ltime;
	struct tm *Tm;
	int sec = -1;
	char buf[BW_LCD_MAX_CHARACTERS];

	for (;;) {
	    ltime = time(NULL);
	    Tm = localtime(&ltime);
	    if (sec != Tm->tm_sec) {
	    	sprintf(buf, "%.2d:%.2d:%.2d", Tm->tm_hour, Tm->tm_min, Tm->tm_sec);
	    	lcd_text_line_2(buf, 8);
	    	sec = Tm->tm_sec;
	    }
    }

	lcd_end();

	return 0;
}
