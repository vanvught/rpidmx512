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

The file functions are implemented on top of the **FatFs - FAT file system module** ([https://github.com/vanvught/rpidmx512/tree/master/lib-ff11](https://github.com/vanvught/rpidmx512/tree/master/lib-ff11 "lib-ff11"))

	FILE *fopen(const char *path, const char *mode)
	int fclose(FILE *fp)
	char *fgets(char *s, int size, FILE *stream)
	int fputs(const char *s, FILE *stream)



*stdlib.h* functions :

	void *malloc(size_t size)
	void free(void *p)
	void *calloc(size_t n, size_t size)
	void *realloc(void *ptr, size_t size)

*math.h* functions :

    float log2f(float)
    float logf(float)

[http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)

