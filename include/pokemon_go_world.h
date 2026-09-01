#ifndef GUARD_POKEMON_GO_WORLD_H
#define GUARD_POKEMON_GO_WORLD_H

void Pgw_InitWorldState(void);
void Pgw_AdvanceWorldDays(u16 days);
void Pgw_CalculateInternalElapsed(u32 realSeconds, struct Time *internalElapsed);
bool32 Pgw_TryConvertRealRtcToSeconds(const struct SiiRtcInfo *rtc, u32 *seconds);
void Pgw_SnapshotWorldClockRealTime(void);
void Pgw_ApplyOfflineWorldClock(void);
void Pgw_CalculateSeasonAfterDays(enum PgwSeason initialSeason, u16 initialDay, u32 elapsedDays, enum PgwSeason *season, u16 *seasonDay);
enum PgwSeasonPhase Pgw_GetSeasonPhaseForDay(u16 seasonDay);
u16 Pgw_CalculateWeatherSeedAfterDays(u16 seed, u32 elapsedDays);
enum PgwClimate Pgw_CalculateClimate(u16 seed, enum PgwSeason season, u16 seasonDay, enum PgwStartingRegion region, u16 locationId, u8 currentHour, u8 hoursAhead);

enum PgwStartingRegion Pgw_GetStartingRegion(void);
void Pgw_SetStartingRegion(enum PgwStartingRegion region);
enum PgwStartingRegion Pgw_GetCurrentRegion(void);
void Pgw_SetCurrentRegion(enum PgwStartingRegion region);
enum PgwSeason Pgw_GetSeason(void);
const u8 *Pgw_GetSeasonName(enum PgwSeason season);
u16 Pgw_GetSeasonDay(void);
enum PgwSeasonPhase Pgw_GetSeasonPhase(void);
enum PgwClimate Pgw_GetCurrentClimate(u16 locationId);
const u8 *Pgw_GetClimateName(enum PgwClimate climate);

void Pgw_ScriptSetStartingRegion(void);
void Pgw_ScriptSetCurrentRegion(void);
void Pgw_ScriptGetStartingRegion(void);
void Pgw_ScriptGetSeason(void);

#endif // GUARD_POKEMON_GO_WORLD_H
