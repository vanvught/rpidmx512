## Open source Raspberry Pi Baremetal C library for standard C functions ##

*ctype.h* functions :
  
	int isdigit(const int c) { return (c >= (int) '0' && c <= (int) '9') ? 1 : 0; }
	int isxdigit(const int c) { return ((isdigit(c) != 0) || (((unsigned) c | 32) - (int) 'a' < 6)) ? 1 : 0; }
	int isprint(const int c) { return ((c >= (int) ' ' && c <= (int) '~')) ? 1 : 0; }
	int isupper(const int c) { return (c >= (int) 'A' && c <= (int) 'Z') ? 1 : 0; }
	int islower(const int c) { return (c >= (int) 'a' && c <= (int) 'z') ? 1 : 0; }
	int isalpha(const int c) { return ((isupper(c) != 0) || (islower(c) != 0)) ? 1 : 0; }
	int tolower(const int c) { return ((isupper(c) != 0) ? (c + 32) : c); }
	int toupper(const int c) { return ((islower(c) != 0) ? (c - 32) : c); }

*string.h* functions :

	int memcmp(const void *s1, const void *s2, size_t n)
	void *memcpy(void *dest, const void *src, size_t n)
	void *memmove(void *dst, const void *src, size_t n)
	void *memset(void *dest, int c, size_t n)

	size_t strlen(const char *s)
	char *strcpy(char *s1, const char *s2)
	char *strncpy(char *s1, const char *s2, size_t n)
	int strcmp(const char *s1, const char *s2)
	int strncmp(const char *s1, const char *s2, size_t n)
	int strcasecmp(const char *s1, const char *s2)
	int strncasecmp(const char *s1, const char *s2, size_t n)

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

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

