	.section .rodata

	.include "asm/macros.inc"
	.include "constants/constants.inc"
	.include "asm/macros/m4a.inc"
	.include "asm/macros/music_voice.inc"
	.include "include/config/general.h"
	.include "include/config/pokemon.h"
#define PC_VOICEGROUP_SOURCE
	.include "sound/voice_groups.inc"

	.align 2
