#include "global.h"
#include "event_data.h"
#include "fake_rtc.h"
#include "pokemon_go_world.h"
#include "rtc.h"

static const u8 sText_SeasonSpring[] = _("SPRING");
static const u8 sText_SeasonSummer[] = _("SUMMER");
static const u8 sText_SeasonAutumn[] = _("AUTUMN");
static const u8 sText_SeasonWinter[] = _("WINTER");

static const u8 *const sSeasonNames[PGW_SEASON_COUNT] =
{
    [PGW_SEASON_SPRING] = sText_SeasonSpring,
    [PGW_SEASON_SUMMER] = sText_SeasonSummer,
    [PGW_SEASON_AUTUMN] = sText_SeasonAutumn,
    [PGW_SEASON_WINTER] = sText_SeasonWinter,
};

static u16 AdvanceWeatherSeed(u16 seed)
{
    return seed * 25173 + 13849;
}

static bool32 GetRealTimeSeconds(u32 *seconds)
{
    struct SiiRtcInfo rtc = {0};

    RtcGetRawInfo(&rtc);
    return Pgw_TryConvertRealRtcToSeconds(&rtc, seconds);
}

static u32 GetRealTimeAnchor(void)
{
    return (u32)VarGet(VAR_PGW_REAL_TIME_ANCHOR_LO)
         | ((u32)VarGet(VAR_PGW_REAL_TIME_ANCHOR_HI) << 16);
}

static void SetRealTimeAnchor(u32 seconds)
{
    VarSet(VAR_PGW_REAL_TIME_ANCHOR_LO, (u16)seconds);
    VarSet(VAR_PGW_REAL_TIME_ANCHOR_HI, (u16)(seconds >> 16));
}

void Pgw_InitWorldState(void)
{
    u16 weatherSeed = gSaveBlock1Ptr->dailySeed ^ (gSaveBlock1Ptr->dailySeed >> 16);

    if (weatherSeed == 0)
        weatherSeed = 1;

    VarSet(VAR_PGW_STARTING_REGION, PGW_DEFAULT_STARTING_REGION);
    VarSet(VAR_PGW_CURRENT_REGION, PGW_DEFAULT_STARTING_REGION);
    VarSet(VAR_PGW_SEASON, PGW_DEFAULT_SEASON);
    VarSet(VAR_PGW_SEASON_DAY, 1);
    VarSet(VAR_PGW_WEATHER_SEED, weatherSeed);
    VarSet(VAR_PGW_WORLD_LEVEL, 0);
    Pgw_SnapshotWorldClockRealTime();
}

void Pgw_SnapshotWorldClockRealTime(void)
{
    u32 realSeconds;

    if (GetRealTimeSeconds(&realSeconds))
        SetRealTimeAnchor(realSeconds);
}

void Pgw_ApplyOfflineWorldClock(void)
{
    struct Time internalElapsed;
    u32 previousSeconds = GetRealTimeAnchor();
    u32 currentSeconds;

    if (!GetRealTimeSeconds(&currentSeconds))
        return;

    SetRealTimeAnchor(currentSeconds);
    if (previousSeconds == 0 || currentSeconds <= previousSeconds)
        return;

    Pgw_CalculateInternalElapsed(currentSeconds - previousSeconds, &internalElapsed);
    FakeRtc_AdvanceTimeBy(internalElapsed.days,
                          internalElapsed.hours,
                          internalElapsed.minutes,
                          internalElapsed.seconds);
}

void Pgw_AdvanceWorldDays(u16 days)
{
    enum PgwSeason season;
    u16 seasonDay;
    u16 seed;

    if (days == 0)
        return;

    season = Pgw_GetSeason();
    seasonDay = Pgw_GetSeasonDay();
    Pgw_CalculateSeasonAfterDays(season, seasonDay, days, &season, &seasonDay);

    VarSet(VAR_PGW_SEASON, season);
    VarSet(VAR_PGW_SEASON_DAY, seasonDay);

    seed = VarGet(VAR_PGW_WEATHER_SEED);
    while (days--)
        seed = AdvanceWeatherSeed(seed);
    VarSet(VAR_PGW_WEATHER_SEED, seed);
}

enum PgwStartingRegion Pgw_GetStartingRegion(void)
{
    u16 region = VarGet(VAR_PGW_STARTING_REGION);

    if (region >= PGW_START_REGION_COUNT)
        return PGW_DEFAULT_STARTING_REGION;
    return region;
}

void Pgw_SetStartingRegion(enum PgwStartingRegion region)
{
    if (region >= PGW_START_REGION_COUNT)
        region = PGW_DEFAULT_STARTING_REGION;

    VarSet(VAR_PGW_STARTING_REGION, region);
    Pgw_SetCurrentRegion(region);
}

enum PgwStartingRegion Pgw_GetCurrentRegion(void)
{
    u16 region = VarGet(VAR_PGW_CURRENT_REGION);

    if (region >= PGW_START_REGION_COUNT)
        return PGW_DEFAULT_STARTING_REGION;
    return region;
}

void Pgw_SetCurrentRegion(enum PgwStartingRegion region)
{
    if (region >= PGW_START_REGION_COUNT)
        region = PGW_DEFAULT_STARTING_REGION;
    VarSet(VAR_PGW_CURRENT_REGION, region);
}

enum PgwSeason Pgw_GetSeason(void)
{
    u16 season = VarGet(VAR_PGW_SEASON);

    if (season >= PGW_SEASON_COUNT)
        return PGW_DEFAULT_SEASON;
    return season;
}

const u8 *Pgw_GetSeasonName(enum PgwSeason season)
{
    if (season >= PGW_SEASON_COUNT)
        season = PGW_DEFAULT_SEASON;
    return sSeasonNames[season];
}

u16 Pgw_GetSeasonDay(void)
{
    u16 seasonDay = VarGet(VAR_PGW_SEASON_DAY);

    if (seasonDay == 0 || seasonDay > PGW_DAYS_PER_SEASON)
        return 1;
    return seasonDay;
}

enum PgwSeasonPhase Pgw_GetSeasonPhase(void)
{
    return Pgw_GetSeasonPhaseForDay(Pgw_GetSeasonDay());
}

void Pgw_ScriptSetStartingRegion(void)
{
    Pgw_SetStartingRegion(gSpecialVar_0x8004);
    gSpecialVar_Result = Pgw_GetStartingRegion();
}

void Pgw_ScriptSetCurrentRegion(void)
{
    Pgw_SetCurrentRegion(gSpecialVar_0x8004);
    gSpecialVar_Result = Pgw_GetCurrentRegion();
}

void Pgw_ScriptGetStartingRegion(void)
{
    gSpecialVar_Result = Pgw_GetStartingRegion();
}

void Pgw_ScriptGetSeason(void)
{
    gSpecialVar_Result = Pgw_GetSeason();
}
