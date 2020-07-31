/*
 * __popcountsi2.c
 *
 */

#include <stdint.h>

uint32_t __popcountsi2(uint32_t x) {
	x = x - ((x >> 1) & 0x55555555);
	/* Every 2 bits holds the sum of every pair of bits */
	x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
	/* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) */
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	/* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) */
	x = (x + (x >> 16));
	/* The lower 16 bits hold two 8 bit sums (5 significant bits).*/
	/*    Upper 16 bits are garbage */
	return (x + (x >> 8)) & 0x0000003F; /* (6 significant bits) */
}
