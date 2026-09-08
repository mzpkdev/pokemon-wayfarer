#if ENABLE_BERRY_GLITCH_FIX_MULTIBOOT
	.section .rodata

gMultiBootProgram_BerryGlitchFix_Start::
	.incbin "data/mb_berry_fix.gba"
gMultiBootProgram_BerryGlitchFix_End::
#endif
