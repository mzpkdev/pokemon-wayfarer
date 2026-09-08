#if ENABLE_COLOSSEUM_MULTIBOOT
	.section .rodata

gMultiBootProgram_PokemonColosseum_Start::
	.incbin "data/mb_colosseum.gba"
gMultiBootProgram_PokemonColosseum_End::
#endif
