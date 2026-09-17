SeafloorCavern_Entrance_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, SeafloorCavern_Entrance_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, SeafloorCavern_Entrance_OnTransition
	.byte 0

SeafloorCavern_Entrance_OnTransition:
	call SeafloorCavern_EventScript_SyncAquaVisibility
	end

SeafloorCavern_EventScript_SyncAquaVisibility::
	special PgrProgress_ScriptCheckHoennSeafloorCavern
	goto_if_eq VAR_RESULT, TRUE, SeafloorCavern_EventScript_ShowAqua
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ENTRANCE_AQUA_GRUNT
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_AQUA_GRUNTS
	return

SeafloorCavern_EventScript_ShowAqua:
	clearflag FLAG_HIDE_SEAFLOOR_CAVERN_AQUA_GRUNTS
	goto_if_set FLAG_RECEIVED_HM_DIVE, SeafloorCavern_EventScript_HideEntranceGrunt
	clearflag FLAG_HIDE_SEAFLOOR_CAVERN_ENTRANCE_AQUA_GRUNT
	return

SeafloorCavern_EventScript_HideEntranceGrunt:
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ENTRANCE_AQUA_GRUNT
	return

SeafloorCavern_Entrance_OnResume:
	setdivewarp MAP_UNDERWATER_SEAFLOOR_CAVERN, 6, 5
	setescapewarp MAP_UNDERWATER_SEAFLOOR_CAVERN, 6, 5
	end

SeafloorCavern_Entrance_EventScript_Grunt::
	lockall
	goto_if_eq VAR_HAS_TALKED_TO_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, 1, SeafloorCavern_Entrance_EventScript_GruntSpeechShort
	waitse
	playse SE_PIN
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_Delay48
	waitmovement 0
	delay 20
	call_if_eq VAR_FACING, DIR_WEST, SeafloorCavern_Entrance_EventScript_GruntFacePlayerWest
	call_if_eq VAR_FACING, DIR_EAST, SeafloorCavern_Entrance_EventScript_GruntFacePlayerEast
	call_if_eq VAR_FACING, DIR_NORTH, SeafloorCavern_Entrance_EventScript_GruntFacePlayerNorth
	delay 30
	setvar VAR_HAS_TALKED_TO_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, 1
	copyobjectxytoperm LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT
	msgbox SeafloorCavern_Entrance_Text_HearMagmaNearMossdeep, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	releaseall
	end

SeafloorCavern_Entrance_EventScript_GruntSpeechShort::
	call_if_eq VAR_FACING, DIR_WEST, SeafloorCavern_Entrance_EventScript_GruntFacePlayerWest
	call_if_eq VAR_FACING, DIR_EAST, SeafloorCavern_Entrance_EventScript_GruntFacePlayerEast
	call_if_eq VAR_FACING, DIR_NORTH, SeafloorCavern_Entrance_EventScript_GruntFacePlayerNorth
	msgbox SeafloorCavern_Entrance_Text_HearMagmaNearMossdeepShort, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	releaseall
	end

SeafloorCavern_Entrance_EventScript_GruntFacePlayerEast::
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	return

SeafloorCavern_Entrance_EventScript_GruntFacePlayerWest::
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	return

SeafloorCavern_Entrance_EventScript_GruntFacePlayerNorth::
	applymovement LOCALID_SEAFLOOR_CAVERN_ENTRANCE_GRUNT, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	return

SeafloorCavern_Entrance_Text_HearMagmaNearMossdeep:
	.string "Hey!\n"
	.string "I remember your face!\p"
	.string "If you're here, it must mean that\n"
	.string "you're about to mess with us again!\p"
	.string "A punk like you, do you really think\n"
	.string "you can take on TEAM AQUA?\p"
	.string "I'd say you're too early by about\n"
	.string "a trillion years!\p"
	.string "You're a perfect fit for the likes of\n"
	.string "TEAM MAGMA!\p"
	.string "Speaking of TEAM MAGMA, I hear they\n"
	.string "were spotted near MOSSDEEP.\p"
	.string "That bunch of goons, they sure don't\n"
	.string "look good near the sea!$"

SeafloorCavern_Entrance_Text_HearMagmaNearMossdeepShort:
	.string "A punk like you, do you really think\n"
	.string "you can take on TEAM AQUA?\p"
	.string "I'd say you're too early by about\n"
	.string "a trillion years!\p"
	.string "You're a perfect fit for the likes of\n"
	.string "TEAM MAGMA!\p"
	.string "Speaking of TEAM MAGMA, I hear they\n"
	.string "were spotted near MOSSDEEP.\p"
	.string "That bunch of goons, they sure don't\n"
	.string "look good near the sea!$"
