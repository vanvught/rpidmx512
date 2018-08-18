/*
	struct { unsigned quot, unsigned rem} __aeabi_uidivmod(unsigned numerator, unsigned denominator) {
		unsigned rem, quot;
		quot = __aeabi_uidiv(numerator, denominator);
		rem = numerator - (quot * denominator)
		return {quot, rem};
	}
*/

	.globl	__aeabi_uidivmod
	.align	4

__aeabi_uidivmod:
	stmfd	sp!, {r0, r1, ip, lr}	/* r0 ← numerator , r1 ← denominator */
	bl	__aeabi_uidiv
	ldmfd	sp!, {r1, r2, ip, lr}	/* r0 ← quot , r1 ← numerator , r2 ← denominator */
	mul	r3, r0, r2
	sub	r1, r1, r3					/* r1 ← rem */
	mov	pc, lr
