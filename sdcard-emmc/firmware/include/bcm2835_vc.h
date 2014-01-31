#ifndef BCM2835_VC_H_
#define BCM2835_VC_H_

#include <stdint.h>

extern uint32_t bcm2835_vc_get_clock_rate(int);
extern uint32_t bcm2835_vc_set_clock_rate(int, uint32_t);

#endif /* BCM2835_VC_H_ */
