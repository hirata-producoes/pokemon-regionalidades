#include "global.h"
#include "pokemon_go_world.h"

#define SECONDS_PER_DAY (HOURS_PER_DAY * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)

static bool32 TryConvertBcd(u8 bcd, u8 *value)
{
    u8 high = bcd >> 4;
    u8 low = bcd & 0xF;

    if (high > 9 || low > 9)
        return FALSE;
    *value = high * 10 + low;
    return TRUE;
}

static bool32 IsRegionalidadesLeapYear(u8 year)
{
    // The cartridge RTC represents 2000-2099, where every offset divisible by
    // four is a leap year (including offset zero, the year 2000).
    return year % 4 == 0;
}

bool32 Pgw_TryConvertRealRtcToSeconds(const struct SiiRtcInfo *rtc, u32 *seconds)
{
    static const u8 sDaysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    u32 dayCount = 0;
    u8 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 i;

    if ((rtc->status & SIIRTCINFO_POWER) || !(rtc->status & SIIRTCINFO_24HOUR))
        return FALSE;
    if (!TryConvertBcd(rtc->year, &year)
     || !TryConvertBcd(rtc->month, &month)
     || !TryConvertBcd(rtc->day, &day)
     || !TryConvertBcd(rtc->hour, &hour)
     || !TryConvertBcd(rtc->minute, &minute)
     || !TryConvertBcd(rtc->second, &second))
        return FALSE;
    if (month < 1 || month > ARRAY_COUNT(sDaysPerMonth)
     || day < 1
     || day > sDaysPerMonth[month - 1] + (month == MONTH_FEB && IsRegionalidadesLeapYear(year))
     || hour >= HOURS_PER_DAY
     || minute >= MINUTES_PER_HOUR
     || second >= SECONDS_PER_MINUTE)
        return FALSE;

    for (i = 0; i < year; i++)
        dayCount += 365 + IsRegionalidadesLeapYear(i);
    for (i = MONTH_JAN; i < month; i++)
        dayCount += sDaysPerMonth[i - 1] + (i == MONTH_FEB && IsRegionalidadesLeapYear(year));
    dayCount += day;

    *seconds = dayCount * SECONDS_PER_DAY
             + hour * MINUTES_PER_HOUR * SECONDS_PER_MINUTE
             + minute * SECONDS_PER_MINUTE
             + second;
    return TRUE;
}

void Pgw_CalculateInternalElapsed(u32 realSeconds, struct Time *internalElapsed)
{
    u32 internalSeconds;

    internalElapsed->days = realSeconds / SECONDS_PER_DAY * PGW_WORLD_CLOCK_REALTIME_MULTIPLIER;
    internalSeconds = realSeconds % SECONDS_PER_DAY * PGW_WORLD_CLOCK_REALTIME_MULTIPLIER;
    internalElapsed->days += internalSeconds / SECONDS_PER_DAY;
    internalSeconds %= SECONDS_PER_DAY;
    internalElapsed->hours = internalSeconds / (MINUTES_PER_HOUR * SECONDS_PER_MINUTE);
    internalSeconds %= MINUTES_PER_HOUR * SECONDS_PER_MINUTE;
    internalElapsed->minutes = internalSeconds / SECONDS_PER_MINUTE;
    internalElapsed->seconds = internalSeconds % SECONDS_PER_MINUTE;
}

void Pgw_CalculateSeasonAfterDays(enum PgwSeason initialSeason, u16 initialDay, u32 elapsedDays, enum PgwSeason *season, u16 *seasonDay)
{
    u32 dayOffset;

    if (initialSeason >= PGW_SEASON_COUNT)
        initialSeason = PGW_DEFAULT_SEASON;
    if (initialDay == 0 || initialDay > PGW_DAYS_PER_SEASON)
        initialDay = 1;

    initialSeason = (initialSeason + elapsedDays / PGW_DAYS_PER_SEASON) % PGW_SEASON_COUNT;
    dayOffset = initialDay - 1 + elapsedDays % PGW_DAYS_PER_SEASON;
    if (dayOffset >= PGW_DAYS_PER_SEASON)
    {
        dayOffset -= PGW_DAYS_PER_SEASON;
        initialSeason = (initialSeason + 1) % PGW_SEASON_COUNT;
    }

    *season = initialSeason;
    *seasonDay = dayOffset + 1;
}

enum PgwSeasonPhase Pgw_GetSeasonPhaseForDay(u16 seasonDay)
{
    if (seasonDay >= 1 && seasonDay <= PGW_SEASON_TRANSITION_HALF_DAYS)
        return PGW_SEASON_PHASE_TRANSITION_IN;
    if (seasonDay > PGW_DAYS_PER_SEASON - PGW_SEASON_TRANSITION_HALF_DAYS
     && seasonDay <= PGW_DAYS_PER_SEASON)
        return PGW_SEASON_PHASE_TRANSITION_OUT;
    return PGW_SEASON_PHASE_ESTABLISHED;
}

u16 Pgw_CalculateWeatherSeedAfterDays(u16 seed, u32 elapsedDays)
{
    if (seed == 0)
        seed = 1;

    while (elapsedDays--)
        seed = seed * 25173 + 13849;
    return seed;
}

static u32 MixClimateValue(u32 value)
{
    // Small deterministic mixer suitable for both GBA and native builds. It
    // never consumes the gameplay RNG, so opening a forecast cannot alter
    // encounters, battles or scripts.
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    value ^= value >> 16;
    return value;
}

// Cumulative prototype weights for clear, cloudy, rain, storm, fog and wind.
// They make the vertical slice readable and intentionally remain tuning data,
// not a frozen ecological model. Biome profiles will refine them later.
static const u8 sClimateThresholds[PGW_SEASON_COUNT][PGW_CLIMATE_COUNT] =
{
    [PGW_SEASON_SPRING] = {35, 60, 82, 88, 96, 100},
    [PGW_SEASON_SUMMER] = {50, 72, 86, 93, 96, 100},
    [PGW_SEASON_AUTUMN] = {30, 60, 80, 87, 95, 100},
    [PGW_SEASON_WINTER] = {30, 65, 80, 84, 94, 100},
};

enum PgwClimate Pgw_CalculateClimate(u16 seed, enum PgwSeason season, u16 seasonDay, enum PgwStartingRegion region, u16 locationId, u8 currentHour, u8 hoursAhead)
{
    u32 absoluteHour = currentHour + hoursAhead;
    u32 elapsedDays = absoluteHour / HOURS_PER_DAY;
    u32 climateSlot = absoluteHour % HOURS_PER_DAY / PGW_CLIMATE_SLOT_HOURS;
    u32 value;
    u32 roll;
    enum PgwClimate climate;

    if (region >= PGW_START_REGION_COUNT)
        region = PGW_DEFAULT_STARTING_REGION;

    seed = Pgw_CalculateWeatherSeedAfterDays(seed, elapsedDays);
    Pgw_CalculateSeasonAfterDays(season, seasonDay, elapsedDays, &season, &seasonDay);

    value = seed;
    value ^= (u32)season << 28;
    value ^= (u32)seasonDay << 20;
    value ^= (u32)region << 16;
    value ^= (u32)locationId << 2;
    value ^= climateSlot;
    roll = MixClimateValue(value) % 100;

    for (climate = PGW_CLIMATE_CLEAR; climate < PGW_CLIMATE_COUNT; climate++)
    {
        if (roll < sClimateThresholds[season][climate])
            return climate;
    }
    return PGW_CLIMATE_CLEAR;
}
