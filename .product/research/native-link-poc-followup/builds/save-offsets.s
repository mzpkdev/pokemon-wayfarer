	.arch armv4t
	.fpu softvfp
	.file	"save-offsets.c"
	.text
	.global	save_offsets
	.section	.rodata
	.align	2
	.type	save_offsets, %object
	.size	save_offsets, 80
save_offsets:
	.word	15760
	.word	3892
	.word	1160
	.word	1380
	.word	2612
	.word	4324
	.word	6540
	.word	304
	.word	6844
	.word	512
	.word	572
	.word	600
	.word	568
	.word	1612
	.word	2272
	.word	34144
	.word	172
	.word	3624
	.word	92
	.word	28
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
