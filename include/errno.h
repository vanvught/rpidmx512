/**
 * @file errno.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ERRNO_H_
#define ERRNO_H_

extern int errno;

/*
errno -l | cut -f1-2 -d ' ' | awk '$0="#define "$0'
*/

#define EOK			0
#define EPERM 		1 /* Operation not permitted */
#define ENOENT	 	2 /* No such file or directory */
#define ESRCH 		3 /* No such process */
#define EINTR 		4 /* Interrupted system call */
#define EIO 		5 /* Input/output error */
#define ENXIO 		6 /* No such device or address */
#define E2BIG 		7 /* Argument list too long */
#define ENOEXEC 	8 /* Exec format error */
#define EBADF 		9 /* Bad file descriptor */
#define ECHILD 		10 /* No child processes */
#define EAGAIN 		11 /* Resource temporarily unavailable */
#define ENOMEM 		12 /* Cannot allocate memory */
#define EACCES 		13 /* Permission denied */
#define EFAULT 		14 /* Bad address */
#define ENOTBLK 	15 /* Block device required */
#define EBUSY 		16 /* Device or resource busy */
#define EEXIST 		17 /* File exists */
#define EXDEV 		18 /* Invalid cross-device link */
#define ENODEV 		19 /* No such device */
#define ENOTDIR	 	20 /* Not a directory */
#define EISDIR 		21 /* Is a directory */
#define EINVAL 		22 /* Invalid argument */
#define ENFILE 		23 /* Too many open files in system */
#define EMFILE 		24 /* Too many open files */
#define ENOTTY 		25 /* Inappropriate ioctl for device */
#define ETXTBSY		26 /* Text file busy */
#define EFBIG 		27 /* File too large */
#define ENOSPC 		28 /* No space left on device */
#define ESPIPE 		29 /* Illegal seek */
#define EROFS 		30 /* Read-only file system */
#define EMLINK 		31 /* Too many links */
#define EPIPE 		32 /* Broken pipe */
#define EDOM 		33 /* Numerical argument out of domain */
#define ERANGE 		34 /* Numerical result out of range */
#define EDEADLK 	35 /* Resource deadlock avoided */
#define ENAMETOOLONG 36 /* File name too long */
#define ENOLCK 		37 /* No locks available */
#define ENOSYS 		38 /* Function not implemented */
#define ENOTEMPTY 	39 /* Directory not empty */
#define EBADMSG 	40 /* Bad message */

#define ELAST		41

#endif /* ERRNO_H_ */
