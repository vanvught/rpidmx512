#ifndef UUID__H_
#define UUID__H_

#include <stdio.h>

struct uuid {
	uint32_t time_low;
	uint16_t time_mid;
	uint16_t time_hi_and_version;
	uint16_t clock_seq;
	uint8_t node[6];
};


#endif /* UUID__H_ */
