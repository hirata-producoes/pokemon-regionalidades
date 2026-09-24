#ifndef GUARD_START_MENU_H
#define GUARD_START_MENU_H

extern bool8 (*gMenuCallback)(void);

#ifdef PLATFORM_SDL2
bool8 Pgr_IsPanelMenuActive(void);
u8 Pgr_GetPanelMenuCount(void);
const u8 *Pgr_GetPanelMenuLabel(u8 index);
u8 Pgr_GetPanelMenuCursor(void);
void Pgr_ClickPanelMenu(u8 index);
#endif

void ShowReturnToFieldStartMenu(void);
void Task_ShowStartMenu(u8 taskId);
void ShowStartMenu(void);
void ShowBattlePyramidStartMenu(void);
void SaveGame(void);
void CB2_SetUpSaveAfterLinkBattle(void);
void SaveForBattleTowerLink(void);
void HideStartMenu(void);
void AppendToList(u8 *list, u8 *pos, u8 newEntry);

#endif // GUARD_START_MENU_H
