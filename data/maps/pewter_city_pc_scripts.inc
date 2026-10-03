@ Validated Kanto chapter for the unified PC campaign. The original FRLG
@ scripts remain in their map directories for the FRLG build.

PewterCity_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PewterCity_PcOnTransition
	.byte 0

PewterCity_PcOnTransition::
	setworldmapflag FLAG_WORLD_MAP_PEWTER_CITY
	end

PewterCity_EventScript_Lass::
	msgbox PewterCity_PcText_Lass, MSGBOX_NPC
	end

PewterCity_EventScript_FatMan::
	msgbox PewterCity_PcText_FatMan, MSGBOX_NPC
	end

PewterCity_EventScript_GymGuide::
	msgbox PewterCity_PcText_GymGuide, MSGBOX_NPC
	end

PewterCity_EventScript_GymSign::
	msgbox PewterCity_PcText_GymSign, MSGBOX_SIGN
	end

PewterCity_EventScript_CitySign::
	msgbox PewterCity_PcText_CitySign, MSGBOX_SIGN
	end

PewterCity_PcText_Lass::
	.string "BROCK specializes in ROCK-type\n"
	.string "POKéMON. Prepare before you enter!$"

PewterCity_PcText_FatMan::
	.string "The POKéMON CENTER and MART are\n"
	.string "open while you explore PEWTER.$"

PewterCity_PcText_GymGuide::
	.string "BROCK is waiting inside the GYM.\n"
	.string "The road east leads to ROUTE 3.$"

PewterCity_PcText_GymSign::
	.string "PEWTER POKéMON GYM\n"
	.string "LEADER: BROCK$"

PewterCity_PcText_CitySign::
	.string "PEWTER CITY\n"
	.string "A STONE GRAY CITY$"

PewterCity_Gym_Frlg_MapScripts::
	.byte 0

PewterCity_Gym_EventScript_Brock::
	trainerbattle_single TRAINER_PGR_KANTO_LEADER_BROCK, PewterCity_PcText_BrockIntro, PewterCity_PcText_BrockDefeat, PewterCity_Gym_PcAfterBrock
	goto_if_unset FLAG_GOT_TM39_FROM_BROCK, PewterCity_Gym_PcGiveTM
	msgbox PewterCity_PcText_BrockAfter, MSGBOX_NPC
	end

PewterCity_Gym_PcAfterBrock::
	setflag FLAG_DEFEATED_BROCK
	goto PewterCity_Gym_PcGiveTM

PewterCity_Gym_PcGiveTM::
	checkitemspace ITEM_TM39
	goto_if_eq VAR_RESULT, FALSE, PewterCity_Gym_PcNoRoom
	giveitem_msg PewterCity_PcText_ReceivedTM39, ITEM_TM39
	setflag FLAG_GOT_TM39_FROM_BROCK
	msgbox PewterCity_PcText_BrockAfter
	release
	end

PewterCity_Gym_PcNoRoom::
	msgbox PewterCity_PcText_NoRoom
	release
	end

PewterCity_Gym_EventScript_Liam::
	trainerbattle_single TRAINER_PGR_KANTO_CAMPER_LIAM, PewterCity_PcText_LiamIntro, PewterCity_PcText_LiamDefeat
	msgbox PewterCity_PcText_LiamAfter, MSGBOX_AUTOCLOSE
	end

PewterCity_Gym_EventScript_GymGuy::
	msgbox PewterCity_PcText_GymGuy, MSGBOX_NPC
	end

PewterCity_Gym_EventScript_GymStatue::
	msgbox PewterCity_PcText_GymSign, MSGBOX_SIGN
	end

PewterCity_PcText_BrockIntro::
	.string "I'm BROCK, PEWTER's GYM LEADER!\p"
	.string "My ROCK-type POKéMON are ready.\n"
	.string "Show me your best!$"

PewterCity_PcText_BrockDefeat::
	.string "You beat me. You earned the\n"
	.string "BOULDERBADGE!$"

PewterCity_PcText_ReceivedTM39::
	.string "{PLAYER} received TM39\n"
	.string "from BROCK.$"

PewterCity_PcText_BrockAfter::
	.string "TM39 contains ROCK TOMB.\p"
	.string "Your next challenge lies beyond\n"
	.string "ROUTE 3, toward MT. MOON.$"

PewterCity_PcText_NoRoom::
	.string "Make room in your BAG and come\n"
	.string "back for TM39.$"

PewterCity_PcText_LiamIntro::
	.string "You're a long way from BROCK!$"

PewterCity_PcText_LiamDefeat::
	.string "That was a solid battle!$"

PewterCity_PcText_LiamAfter::
	.string "BROCK is waiting farther in.$"

PewterCity_PcText_GymGuy::
	.string "A GRASS- or WATER-type move can\n"
	.string "help against BROCK's ROCK team.$"

PewterCity_Mart_Frlg_MapScripts::
	.byte 0

PewterCity_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart PewterCity_PcMartItems
	msgbox gText_PleaseComeAgain
	release
	end

	.align 2
PewterCity_PcMartItems::
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_NONE

PewterCity_PokemonCenter_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PewterCity_PcCenterOnTransition
	.byte 0

PewterCity_PcCenterOnTransition::
	setrespawn HEAL_LOCATION_PEWTER_CITY
	end

PewterCity_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end
