#ifndef GUARD_POKEMON_REGIONALIDADES_DEX_H
#define GUARD_POKEMON_REGIONALIDADES_DEX_H

enum PgwDexObservation
{
    PGW_DEX_NOT_OBSERVED,
    PGW_DEX_SEEN,
    PGW_DEX_CAUGHT,
};

void PgwDex_ResetResearch(void);
bool32 PgwDex_OnSaveLoaded(void);
bool32 PgwDex_StageNativeState(void);
void PgwDex_UnlockRegion(enum PgwStartingRegion region);
bool32 PgwDex_IsRegionUnlocked(enum PgwStartingRegion region);
u16 PgwDex_GetUnlockedRegionMask(void);
enum PgwDexObservation PgwDex_GetObservation(enum NationalDexOrder nationalNum);

#endif // GUARD_POKEMON_REGIONALIDADES_DEX_H
