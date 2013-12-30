#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

extern void memory_barrier();

inline static void mmio_write(uint32_t reg, uint32_t data)
{
	//memory_barrier();
	*(volatile uint32_t *)(reg) = data;
	//memory_barrier();
}

inline static uint32_t mmio_read(uint32_t reg)
{
	//memory_barrier();
	return *(volatile uint32_t *)(reg);
	//memory_barrier();
}

#endif // !MMIO_H

