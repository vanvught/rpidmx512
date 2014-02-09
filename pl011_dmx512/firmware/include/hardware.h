#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>

void led_set(int state);
void led_init(void);

void __disable_fiq(void);
void __enable_fiq(void);

void __disable_irq(void);
void __enable_irq(void);

extern void watchdog_init(void);
extern void watchdog_feed(void);
extern void watchdog_stop(void);

#endif /* HARDWARE_H_ */
