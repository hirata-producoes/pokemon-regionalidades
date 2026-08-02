PalletTown_Pgw_MapScripts::
	.byte 0

PalletTown_Pgw_EventScript_ReturnToHoenn::
	lock
	faceplayer
	msgbox PalletTown_Pgw_Text_ReturnToHoenn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_Pgw_EventScript_Stay
	setvar VAR_0x8004, 2 @ PGW_START_HOENN
	special Pgw_ScriptSetCurrentRegion
	warp MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 4, 10
	waitstate
	release
	end

PalletTown_Pgw_EventScript_Stay::
	msgbox PalletTown_Pgw_Text_Stay, MSGBOX_DEFAULT
	release
	end

PalletTown_Pgw_EventScript_TownSign::
	msgbox PalletTown_Pgw_Text_TownSign, MSGBOX_SIGN
	end

PalletTown_Pgw_Text_ReturnToHoenn:
	.string "Esta e PALLET TOWN, em KANTO!\p"
	.string "Quer voltar ao laboratorio do\n"
	.string "PROF. BIRCH, em HOENN?$"

PalletTown_Pgw_Text_Stay:
	.string "Tudo bem. Aproveite KANTO!$"

PalletTown_Pgw_Text_TownSign:
	.string "PALLET TOWN\n"
	.string "Uma pequena cidade com um grande futuro.$"
