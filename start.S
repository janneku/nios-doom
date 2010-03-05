.global _start
_start:
	/* Asetetaan pino */
	movhi	sp, %hi(stack+(4*64*1024))
	ori	sp, sp, %lo(stack+(4*64*1024))
	
	call	main
	
	/* jos mainista palataan... */
afterlife:
	br	afterlife

.global FixedMul
FixedMul:
	andi	r8,r4,65535
	andi	r9,r5,65535
	srai	r10,r4,16
	srai	r11,r5,16
	mul	r12,r8,r9 /* ala */
	mul	r13,r10,r11 /* ylä */
	mul	r2,r8,r11 /* ristiin */
	mul	r3,r9,r10 /* ristiin */
	srli	r12,r12,16
	slli	r13,r13,16
	add	r2,r2,r3
	add	r2,r2,r12
	add	r2,r2,r13
	ret

.end
