.cpu arm1176jzf-s
.fpu vfp

.align	2

.globl qmemcpy
qmemcpy:					@ void *qmemcpy(void *dest, void *src, size_t n)
	tst r0, #15
	bne .Lmemcpy
	tst r1, #15
	bne .Lmemcpy
	tst r2, #15
	bne .Lmemcpy
.globl quick_memcpy
quick_memcpy:
	push 	{r0, r6-r9, lr}
.loopb:
		ldmia	r1!, {r6-r9}
		stmia	r0!, {r6-r9}
		subs	r2, #16
		bhi	.loopb
	pop	{r0, r6-r9, pc}
.Lmemcpy:
	b memcpy
