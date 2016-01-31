/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>

unsigned int heap_end = 0x00;

/* Forward prototypes.  */
int     _system     _PARAMS ((const char *));
int     _rename     _PARAMS ((const char *, const char *));
clock_t _times      _PARAMS ((struct tms *));
int     _gettimeofday   _PARAMS ((struct timeval *, struct timezone *));
void    _raise      _PARAMS ((void));
int     _unlink     _PARAMS ((void));
int     _link       _PARAMS ((void));
int     _stat       _PARAMS ((const char *, struct stat *));
int     _fstat      _PARAMS ((int, struct stat *));
caddr_t _sbrk       _PARAMS ((int));
int     _getpid     _PARAMS ((int));
int     _kill       _PARAMS ((int, int));
void    _exit       _PARAMS ((int));
int     _close      _PARAMS ((int));
int     _open       _PARAMS ((const char *, int, ...));
int     _write      _PARAMS ((int, char *, int));
int     _lseek      _PARAMS ((int, int, int));
int     _read       _PARAMS ((int, char *, int));
void    initialise_monitor_handles _PARAMS ((void));

/**
 *
 */
void initialise_monitor_handles(void) {
}

/**
 *
 * @param file
 * @param ptr
 * @param len
 * @return
 */
int _read(int file, char * ptr, int len) {
	return len;
}

/**
 *
 * @param file
 * @param ptr
 * @param dir
 * @return
 */
int _lseek(int file, int ptr, int dir) {
	return 0;
}

#ifdef ENABLE_FRAMEBUFFER
extern int console_putc(int c);
#else
extern void bcm2835_uart_send(unsigned int c);
#endif

/**
 *
 * @param file
 * @param ptr
 * @param len
 * @return
 */
int _write(int file, char * ptr, int len) {
	int r;
	for (r = 0; r < len; r++) {
		unsigned int ch = ptr[r];
#ifndef ENABLE_FRAMEBUFFER
		if (ch == '\n')
			bcm2835_uart_send('\r');
#endif
#ifdef ENABLE_FRAMEBUFFER
		console_putc(ch);
#else
		bcm2835_uart_send(ch);
#endif
	}
	return len;
}

/**
 *
 * @param path
 * @param flags
 * @return
 */
int _open(const char * path, int flags, ...) {
	return 0;
}

/**
 *
 * @param file
 * @return
 */
int _close(int file) {
	return 0;
}

/**
 *
 * @param n
 */
void _exit(int n) {
	while (1)
		;
}

/**
 *
 * @param n
 * @param m
 * @return
 */
int _kill(int n, int m) {
	return (0);
}

/**
 *
 * @param n
 * @return
 */
int _getpid(int n) {
	return 1;
}

/**
 *
 * @param incr
 * @return
 */
caddr_t _sbrk(int incr) {
	extern char heap_low; /* Defined by the linker */
	extern char heap_top; /* Defined by the linker */

	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = (unsigned int)&heap_low;
	}
	prev_heap_end = (char *)heap_end;

	if (heap_end + incr > (unsigned int)&heap_top) {
		/* Heap and stack collision */
		return (caddr_t) 0;
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}

/**
 *
 * @param file
 * @param st
 * @return
 */
int _fstat(int file, struct stat * st) {
	return 0;
}

/**
 *
 * @param fname
 * @param st
 * @return
 */
int _stat(const char *fname, struct stat *st) {
	return 0;
}

/**
 *
 * @return
 */
int _link(void) {
	return -1;
}

/**
 *
 * @return
 */
int _unlink(void) {
	return -1;
}

/**
 *
 */
void _raise(void) {
	return;
}

/**
 *
 * @param tp
 * @param tzp
 * @return
 */
int _gettimeofday(struct timeval * tp, struct timezone * tzp) {
	if (tp) {
		tp->tv_sec = 10;
		tp->tv_usec = 0;
	}
	if (tzp) {
		tzp->tz_minuteswest = 0;
		tzp->tz_dsttime = 0;
	}
	return 0;
}

/**
 *
 * @param tp
 * @return
 */
clock_t _times(struct tms * tp) {
	clock_t timeval;

	timeval = 10;
	if (tp) {
		tp->tms_utime = timeval; /* user time */
		tp->tms_stime = 0; /* system time */
		tp->tms_cutime = 0; /* user time, children */
		tp->tms_cstime = 0; /* system time, children */
	}
	return timeval;
}

/**
 *
 * @param fd
 * @return
 */
int _isatty(int fd) {
	return 1;
}

/**
 *
 * @param s
 * @return
 */
int _system(const char *s) {
	if (s == NULL)
		return 0;
	errno = ENOSYS;
	return -1;
}

/**
 *
 * @param oldpath
 * @param newpath
 * @return
 */
int _rename(const char * oldpath, const char * newpath) {
	errno = ENOSYS;
	return -1;
}
