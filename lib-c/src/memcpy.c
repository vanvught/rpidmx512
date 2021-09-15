#include <stddef.h>

void* memcpy(void *__restrict__ dest, const void *__restrict__ src, size_t n) {
	char *dp = (char *) dest;
	const char *sp = (const char *) src;

	while (n--) {
		*dp++ = *sp++;
	}

	return dest;
}
