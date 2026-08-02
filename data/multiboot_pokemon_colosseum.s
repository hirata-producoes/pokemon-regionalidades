	.section .rodata

gMultiBootProgram_PokemonColosseum_Start::
#ifndef PORTABLE
	.incbin "data/mb_colosseum.gba"
#endif
gMultiBootProgram_PokemonColosseum_End::
