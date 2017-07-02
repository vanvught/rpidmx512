/*
	This routine performs a 32-bit divide of two unsigned integers.
 	Returns the result of the division, or 0 if there was an error (such as divide by 0).

 	unsigned int __aeabi_uidiv(unsigned int num, unsigned int den)
 */

	.globl	__aeabi_uidiv
	.align	4

__aeabi_uidiv:
	clz  r3, r0             	/* r3 ← CLZ(r0) Count leading zeroes of num */
    clz  r2, r1             	/* r2 ← CLZ(r1) Count leading zeroes of den */
    rsb	r3, r3, r2				/* r3 ← r2 - r3 */
    mov r2, #0              	/* r2 ← 0 */
    cmp	r3, #0
  	blt .Lquit
    .Lloop:
      cmp r0, r1, lsl r3      	/* Compute r0 - (r1 << r3) and update cpsr */
      adc r2, r2, r2          	/* r2 ← r2 + r2 + C. Note that if r0 >= (r1 << r3) then C=1, C=0 otherwise */
      subcs r0, r0, r1, lsl r3	/* r0 ← r0 - (r1 << r3) if C = 1 (this is, only if r0 >= (r1 << r3) ) */
      subs r3, r3, #1         	/* r3 ← r3 - 1 */
      bpl .Lloop				/* if r3 >= 0 (N=0) then branch to .Lloop */
.Lquit:
    mov r0, r2
    bx lr
