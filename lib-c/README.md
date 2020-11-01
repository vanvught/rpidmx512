## Open source C library for standard C functions

*assert.h* function :

	void __assert_func(const char *file, int line, const char *func, const char *failedexpr);

*stdio.h* functions :

	int printf(const char* fmt, ...);
	int vprintf(const char *fmt, va_list arp);
	int sprintf(char *str, const char *fmt, ...);
	int vsprintf(char *str, const char *fmt, va_list ap);
	int snprintf(char *str, size_t size, const char *fmt, ...);
	int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
	void perror(const char *s);
	char *strerror(int errnum);

*stdlib.h* functions :

	void *malloc(size_t size);
	void free(void *p);
	void *calloc(size_t n, size_t size);
	void *realloc(void *ptr, size_t size);
	long int random(void);
	void srandom(unsigned int seed);

*time.h* functions :

	time_t time(time_t *t);
	time_t mktime(struct tm *tm);
	struct tm *localtime(const time_t *timep);
	char *asctime(const struct tm *tm);
    int clock_getres(clockid_t clockid, struct timespec *res);
	int clock_gettime(clockid_t clockid, struct timespec *tp);
	int clock_settime(clockid_t clockid, const struct timespec *tp);

*sys/time.h* functions :
	
	int gettimeofday(struct timeval *tv, struct timezone *tz);
	int settimeofday(const struct timeval *tv, const struct timezone *tz);

*math.h* functions :

    float log2f(float);
    float logf(float);

*netinet/in.h*
*arpa/inet.h* functions :

	int inet_aton(const char *cp, struct in_addr *inp);


[http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)
