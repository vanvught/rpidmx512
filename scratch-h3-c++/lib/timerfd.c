/**
 * @file timerfd.c
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

#include <sys/timerfd.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include "../include/unistd.h"
#include <errno.h>

#include "h3.h"

#include "debug.h"

#define MAX_HANDLES	8

struct timerfd_ctx {
	int clockid;
	uint32_t micros;
	int flags;
	struct timespec tp;
	struct itimerspec timerspec;
	uint64_t ticks;
	int expired;
	int one_shot;
};

static struct timerfd_ctx ctx[MAX_HANDLES];
static int32_t handle_count = 0;

int timerfd_create(int clockid, __attribute__((unused)) int flags) {
	if (handle_count == MAX_HANDLES) {
		errno = ENFILE;
		return -1;
	}

	if ((clockid != CLOCK_MONOTONIC && clockid != CLOCK_REALTIME)) {
		errno = ENFILE;
		return -1;
	}

	memset(&ctx[handle_count], 0, sizeof(struct timerfd_ctx));
	ctx[handle_count].clockid = clockid;

	return handle_count++;
}

/**
 * The new_value argument specifies the initial expiration and interval for the timer.
 *
 * If both fields of new_value.it_interval are zero, the timer expires just once,
 * at the time specified by new_value.it_value.
 */
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, __attribute__((unused)) struct itimerspec *old_value) {
	struct timerfd_ctx *c = &ctx[fd];

	c->flags = flags;
	clock_gettime(c->clockid, &c->tp);
	memcpy(&c->timerspec, new_value, sizeof(struct itimerspec));
	c->one_shot = ((new_value->it_interval.tv_sec == 0) && (new_value->it_interval.tv_nsec == 0));

	return 0;
}

static int _timerfd_poll(int fd, void *buf, size_t count, __attribute__((unused)) int timeout) {
	if (count != sizeof(uint64_t)) {
		errno = EINVAL;
		return -1;
	}

	struct timerfd_ctx *c = &ctx[fd];

	if ((c->expired == 1) && (c->one_shot == 1)) {
		errno = EINVAL;
		return -1;
	}

	struct timespec tp;

	clock_gettime(c->clockid, &tp);

	if (c->flags & TFD_TIMER_ABSTIME) {
		if ((c->expired == 0) && (tp.tv_sec >= c->timerspec.it_value.tv_sec)) {
			c->expired = 1;

			if (c->one_shot == 0) {
				c->ticks = (uint32_t)(tp.tv_sec - c->timerspec.it_value.tv_sec) /  (uint32_t) (c->timerspec.it_interval.tv_sec);
			} else {
				c->ticks = 1;
			}

			memcpy(buf, &c->ticks, sizeof(uint64_t));
			return sizeof(uint64_t);
		}

		if (tp.tv_sec >= c->timerspec.it_value.tv_sec){
			c->ticks = 1 + (uint32_t)(tp.tv_sec - c->timerspec.it_value.tv_sec) /  (uint32_t) (c->timerspec.it_interval.tv_sec);
		} else {
			while (tp.tv_sec < c->timerspec.it_value.tv_sec) {
				clock_gettime(c->clockid, &tp);
			}
			c->ticks = 1;
		}

		if (c->one_shot == 0) {
			c->timerspec.it_value.tv_sec += c->timerspec.it_interval.tv_sec;
		}

		memcpy(buf, &c->ticks, sizeof(uint64_t));
		return sizeof(uint64_t);
	} else {
		const time_t elapsed = tp.tv_sec - c->tp.tv_sec;

		if (c->expired == 0) {
			if (elapsed >= c->timerspec.it_value.tv_sec) {
				if (c->one_shot == 0) {
					c->ticks = (uint32_t) (elapsed) / (uint32_t) (c->timerspec.it_interval.tv_sec);
				} else {
					c->ticks = 1;
				}
			} else {
				while ((tp.tv_sec - c->tp.tv_sec) < c->timerspec.it_value.tv_sec) {
					clock_gettime(c->clockid, &tp);
				}
				c->ticks = 1;
			}

			c->expired = 1;
			c->tp.tv_sec = tp.tv_sec;
			memcpy(buf, &c->ticks, sizeof(uint64_t));
			return sizeof(uint64_t);
		}

		while ((tp.tv_sec - c->tp.tv_sec) < c->timerspec.it_interval.tv_sec) {
			clock_gettime(c->clockid, &tp);
		}

		c->ticks = (uint32_t)(tp.tv_sec - c->tp.tv_sec) /  (uint32_t) (c->timerspec.it_interval.tv_sec);
		c->tp.tv_sec = tp.tv_sec;

		memcpy(buf, &c->ticks, sizeof(uint64_t));
		return sizeof(uint64_t);
	}

	return -1;
}

int timerfd_read(int fd, void *buf, size_t count) {
	return _timerfd_poll(fd, buf, count, -1);
}

int timerfd_poll(int fd, int timeout) {
	uint64_t buf;
	return _timerfd_poll(fd, &buf, sizeof(uint64_t), timeout);
}
