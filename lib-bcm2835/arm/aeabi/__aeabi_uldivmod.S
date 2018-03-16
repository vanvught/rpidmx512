/*
	struct { uint64_t quot, uint64_t rem} __aeabi_uldivmod(uint64_t numerator, uint64_t denominator) {
		uint64_t rem, quot;
		quot = udivmoddi4(numerator, denominator, &rem);
		return {quot, rem};
	}
*/
	.globl	__aeabi_uldivmod
	.align	4

__aeabi_uldivmod:
        push	{r11, lr}
        sub	sp, sp, #16
        add	r12, sp, #8
        str	r12, [sp]
        bl	udivmoddi4
        ldr	r2, [sp, #8]
        ldr	r3, [sp, #12]
        add	sp, sp, #16
        pop	{r11, pc}
