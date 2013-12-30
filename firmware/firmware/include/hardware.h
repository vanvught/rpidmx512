#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>

void led_set(int state);
void led_init(void);

void watchdog_init(void);
void watchdog_feed(void);

void __disable_fiq(void);
void __enable_fiq(void);

void __disable_irq(void);
void __enable_irq(void);

#endif /* HARDWARE_H_ */
