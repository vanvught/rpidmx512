## Open source C library for standard C functions

*assert.h* function :

	void __assert_func(const char *file, int line, const char *func, const char *failedexpr)

*stdio.h* functions :

	int printf(const char* fmt, ...)
	int vprintf(const char *fmt, va_list arp)
	int sprintf(char *str, const char *fmt, ...)
	int vsprintf(char *str, const char *fmt, va_list ap)
	int snprintf(char *str, size_t size, const char *fmt, ...)
	int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)

*stdlib.h* functions :

	void *malloc(size_t size)
	void free(void *p)
	void *calloc(size_t n, size_t size)
	void *realloc(void *ptr, size_t size)

*time.h* functions :

	time_t time(time_t *t);
	time_t mktime(struct tm *tm);
	struct tm *localtime(const time_t *timep);
	char *asctime(const struct tm *tm);

*math.h* functions :

    float log2f(float)
    float logf(float)

[http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)
