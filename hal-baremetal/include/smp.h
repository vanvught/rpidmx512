#ifndef SMP_H_
#define SMP_H_

#include <stdint.h>

typedef void (*start_fn_t)(void);

extern uint32_t smp_get_core_number(void);
extern void smp_start_core(uint32_t, start_fn_t);

#endif /* SMP_H_ */
