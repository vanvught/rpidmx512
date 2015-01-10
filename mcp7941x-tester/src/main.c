#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// RTC include
#include <mcp7941x.h>

int main(int argc, char **argv) {
	// RTC time
	struct rtc_time tm_rtc, tm_rtc_set;
	// Linux current time
	time_t ltime;
	struct tm *tm_linux;

	printf("rtc_start\n");
	if (mcp7941x_start(0) != MCP7941X_OK) {
		printf("Cannot start I2C_RTC\n");
		exit(EXIT_FAILURE);
	}

	// Get current localtime from Linux
	ltime = time(NULL);
	tm_linux = localtime(&ltime);

	printf("Linux time:\n\tsecs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d\n", tm_linux->tm_sec, tm_linux->tm_min, tm_linux->tm_hour, tm_linux->tm_mday, tm_linux->tm_mon, tm_linux->tm_year, tm_linux->tm_wday);

	tm_rtc_set.tm_sec = tm_linux->tm_sec;
	tm_rtc_set.tm_min = tm_linux->tm_min;
	tm_rtc_set.tm_hour = tm_linux->tm_hour;
	tm_rtc_set.tm_mday = tm_linux->tm_mday;
	tm_rtc_set.tm_mon = tm_linux->tm_mon;
	tm_rtc_set.tm_year = tm_linux->tm_year - 100; //TODO remove -100
	tm_rtc_set.tm_wday = tm_linux->tm_wday;

	mcp7941x_set_date_time(&tm_rtc_set);

	mcp7941x_get_date_time(&tm_rtc);

	printf("RTC time:\n\tsecs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d\n", tm_rtc.tm_sec, tm_rtc.tm_min, tm_rtc.tm_hour, tm_rtc.tm_mday, tm_rtc.tm_mon, tm_rtc.tm_year, tm_rtc.tm_wday);

	printf("rtc_end\n");
	mcp7941x_end();

	return EXIT_SUCCESS;
}
