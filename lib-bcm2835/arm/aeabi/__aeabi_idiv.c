/*
	This routine performs a 32-bit divide of two signed integers.
	Returns the result of the division, or 0 if there was an error (such as divide by 0).
 */

extern unsigned int __aeabi_uidiv(unsigned int num, unsigned int den);

signed int __aeabi_idiv(signed int num, signed int den) {
	signed int minus = 0;
	signed int v;

	if (num < 0) {
		num = -num;
		minus = 1;
	}

	if (den < 0) {
		den = -den;
		minus ^= 1;
	}

	v = __aeabi_uidiv((unsigned int) num, (unsigned int) den);

	if (minus != 0) {
		v = -v;
	}

	return v;
}
