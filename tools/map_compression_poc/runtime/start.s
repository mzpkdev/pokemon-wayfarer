	.syntax unified
	.arm
	.section .text.start, "ax", %progbits
	.global _start
	.type _start, %function
_start:
	b start
	.space 0xBC
start:
	mov r0, #0xd2
	msr cpsr_c, r0
	ldr sp, =0x03007fa0
	mov r0, #0xdf
	msr cpsr_c, r0
	ldr sp, =0x03007e40
	ldr r0, =__iwram_lma
	ldr r1, =__iwram_start
	ldr r2, =__iwram_end
1:
	cmp r1, r2
	beq 2f
	ldr r3, [r0], #4
	str r3, [r1], #4
	b 1b
2:
	ldr r0, =main + 1
	bx r0
1:
	b 1b

    .thumb
    .align 2
    .global PaintStack
    .thumb_func
PaintStack:
    mov r0, sp
    ldr r1, =gPocStackTop
    str r0, [r1]
    ldr r1, =0x030063d0
    ldr r2, =0xcdcdcdcd
3:
    cmp r1, r0
    bhs 4f
    str r2, [r1]
    adds r1, #4
    b 3b
4:
    bx lr
    .global ReadStack
    .thumb_func
ReadStack:
    ldr r0, =gPocStackTop
    ldr r0, [r0]
    ldr r1, =0x030063d0
    ldr r2, =0xcdcdcdcd
5:
    cmp r1, r0
    bhs 6f
    ldr r3, [r1]
    cmp r3, r2
    bne 6f
    adds r1, #4
    b 5b
6:
    subs r0, r0, r1
    bx lr
