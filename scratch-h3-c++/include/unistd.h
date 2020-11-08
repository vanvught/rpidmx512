/*
 * unistd.h
 *
 *  Created on: 24 okt. 2020
 *      Author: arjanvanvught
 */

#ifndef UNISTD_H_
#define UNISTD_H_

typedef uint64_t	ssize_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int read(int fd, void *buf, size_t count);

#ifdef __cplusplus
}
#endif


#endif /* UNISTD_H_ */
