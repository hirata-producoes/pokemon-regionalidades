	.section .rodata

	.align 2
gMultiBootProgram_EReader_Start::
#ifndef PORTABLE
	.incbin "data/mb_ereader.gba"
#endif
gMultiBootProgram_EReader_End::
