#include <stdint.h>
#include <stddef.h>

uint64_t udivmoddi4(uint64_t num, uint64_t den, uint64_t *rem_p) {
	uint64_t quot = 0, qbit = 1;
	unsigned int shift = 0;

	if (den == 0) {
		return 0;
	}

	shift = __builtin_clzll(den);

	den <<= shift;
	qbit <<= shift;

	while (qbit != (uint32_t) 0) {
		if (den <= num) {
			num -= den;
			quot += qbit;
		}

		den >>= 1;
		qbit >>= 1;
	}

	if (rem_p != NULL) {
		*rem_p = num;
	}

	return quot;
}

