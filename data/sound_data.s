	.section .rodata

	.include "asm/macros.inc"
	.include "constants/constants.inc"

	.include "asm/macros/m4a.inc"
	.include "asm/macros/music_voice.inc"
	.include "include/config/general.h"
	.include "include/config/pokemon.h"
#ifdef PORTABLE
	.include "build/pc-generated/voicegroup_placeholders.inc"
	.include "sound/cry_tables.inc"
	.include "build/pc-generated/voicegroup_exports.inc"
#else
	.include "sound/voice_groups.inc"
#endif
	.include "sound/keysplit_tables.inc"
	.include "sound/programmable_wave_data.inc"
	.include "sound/music_player_table.inc"
	.include "sound/song_table.inc"
#ifdef PORTABLE
	.include "build/pc-generated/music_sample_placeholders.inc"
#else
	.include "sound/direct_sound_data.inc"
#endif

	.align 2
