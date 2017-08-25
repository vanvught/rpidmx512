#include <stdint.h>
#include <time.h>
#include <sys/time.h>

uint32_t millis(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * (__time_t) 1000) + (tv.tv_usec / (__suseconds_t) 1000);
}

