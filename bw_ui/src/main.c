#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <bw_ui.h>

int main(int argc, char **argv) {

	printf("ui_start\n");
	if (ui_start(0x00) != BW_UI_OK) {
		printf("Cannot start LCD\n");
		exit(EXIT_FAILURE);
	}

	printf("ui_read_id\n");
	ui_read_id();

	printf("ui_reinit\n");
	ui_reinit();

	printf("ui_cls\n");
	ui_cls();

	printf("ui_text_line_1\n");
	ui_text_line_1("Raspberry Pi", 12);

	printf("ui_text_line_2\n");
	time_t ltime;
	struct tm *Tm;
	int sec = -1;
	char buf[BW_UI_MAX_CHARACTERS];

	for (;;) {
	    ltime = time(NULL);
	    Tm = localtime(&ltime);
	    if (sec != Tm->tm_sec) {
	    	sprintf(buf, "%.2d:%.2d:%.2d", Tm->tm_hour, Tm->tm_min, Tm->tm_sec);
	    	ui_text_line_2(buf, 8);
	    	sec = Tm->tm_sec;
	    }
    }

	ui_end();

	return 0;
}
