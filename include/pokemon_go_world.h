#ifndef GUARD_POKEMON_GO_WORLD_H
#define GUARD_POKEMON_GO_WORLD_H

void Pgw_InitWorldState(void);
void Pgw_AdvanceWorldDays(u16 days);

enum PgwStartingRegion Pgw_GetStartingRegion(void);
void Pgw_SetStartingRegion(enum PgwStartingRegion region);
enum PgwStartingRegion Pgw_GetCurrentRegion(void);
void Pgw_SetCurrentRegion(enum PgwStartingRegion region);
enum PgwSeason Pgw_GetSeason(void);

void Pgw_ScriptSetStartingRegion(void);
void Pgw_ScriptSetCurrentRegion(void);
void Pgw_ScriptGetStartingRegion(void);
void Pgw_ScriptGetSeason(void);

#endif // GUARD_POKEMON_GO_WORLD_H
