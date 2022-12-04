#include <stddef.h>

void *memmove(void *dst, const void *src, size_t n) {
	char *dp = (char *) dst;
	const char *sp = (const char *) src;

	if (dp < sp) {
		while (n--) {
			*dp++ = *sp++;
		}
	} else {
		sp += n;
		dp += n;
		while (n--) {
			*--dp = *--sp;
		}
	}

	return dst;
}
