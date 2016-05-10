//
// oscutil.cpp
//
#include "oscutil.h"
#include <circle/logger.h>
#include <circle/stdarg.h>
#include <assert.h>

void *calloc (size_t nmemb, size_t size)
{
	size_t nbytes = nmemb * size;
	if (nbytes == 0)
	{
		return 0;
	}

	void *blk = malloc (nbytes);
	assert (blk != 0);

	memset (blk, 0, nbytes);
	
	return blk;
}

void *realloc (void *ptr, size_t size)		// TODO: inefficient
{
	//assert (ptr != 0); //->  If ptr is NULL, then the call is equivalent to malloc(size)

	if (ptr == 0)
	{
		void *newblk = malloc(size);
		assert(newblk != 0);
		return newblk;
	}

	if (size == 0)
	{
		free (ptr);

		return 0;
	}

	void *newblk = malloc (size);
	assert (newblk != 0);

	memcpy (newblk, ptr, size);

	free (ptr);

	return newblk;
}

void printf (const char *fmt, ...)
{
	assert (fmt != 0);

	// remove trailing '\n'
	size_t fmtlen = strlen (fmt);
	char fmtbuf[fmtlen+1];

	strcpy (fmtbuf, fmt);

	if (fmtbuf[fmtlen] == '\n')
	{
		fmtbuf[fmtlen] = '\0';
	}
	
	va_list var;
	va_start (var, fmt);

	CLogger::Get ()->WriteV ("osc", LogDebug, fmtbuf, var);

	va_end (var);
}
