#ifndef SMP_H_
#define SMP_H_

#include <stdint.h>

#define SMP_CORE_MASK 			3			///<
#define SMP_CORE_BASE			0x4000008C	///<

typedef void (*start_fn_t)(void);
extern void _init_core(void);

extern uint32_t smp_get_core_number(void);
#if defined (ARM_ALLOW_MULTI_CORE)
extern void smp_start_core(uint32_t, start_fn_t);
#endif
#endif /* SMP_H_ */
