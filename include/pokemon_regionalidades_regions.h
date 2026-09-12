#ifndef GUARD_POKEMON_REGIONALIDADES_REGIONS_H
#define GUARD_POKEMON_REGIONALIDADES_REGIONS_H

#include "global.h"
#include "constants/pokemon_go_world.h"

enum PgrRegionAvailability
{
    PGR_REGION_PLANNED,
    PGR_REGION_MAP_DATA,
    PGR_REGION_PLAYABLE,
};

enum PgrRegionIntro
{
    PGR_REGION_INTRO_NONE,
    PGR_REGION_INTRO_HOENN,
};

struct PgrRegionEntryPoint
{
    s8 mapGroup;
    s8 mapNum;
    s8 warpId;
    s8 x;
    s8 y;
};

struct PgrRegionDefinition
{
    enum PgwStartingRegion id;
    const u8 *name;
    enum PgrRegionAvailability availability;
    enum PgrRegionIntro intro;
    struct PgrRegionEntryPoint entryPoint;
};

const struct PgrRegionDefinition *Pgr_GetWorldRegion(u32 index);
const struct PgrRegionDefinition *Pgr_FindWorldRegion(enum PgwStartingRegion region);
bool32 Pgr_IsWorldRegion(enum PgwStartingRegion region);
bool32 Pgr_CanStartAdventureInRegion(enum PgwStartingRegion region);
const struct PgrRegionEntryPoint *Pgr_GetRegionEntryPoint(enum PgwStartingRegion region);

#endif // GUARD_POKEMON_REGIONALIDADES_REGIONS_H
