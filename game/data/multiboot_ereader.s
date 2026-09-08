#if ENABLE_EREADER_TRANSFER
	.section .rodata

	.align 2
gMultiBootProgram_EReader_Start::
	.incbin "data/mb_ereader.gba"
gMultiBootProgram_EReader_End::

#endif
