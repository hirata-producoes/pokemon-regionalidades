#include "global.h"
#include "pokemon_regionalidades_regions.h"
#include "constants/map_groups.h"

static const u8 sText_Kanto[] = _("KANTO");
static const u8 sText_Johto[] = _("JOHTO");
static const u8 sText_Hoenn[] = _("HOENN");
static const u8 sText_Sinnoh[] = _("SINNOH");

// This registry is the single source of truth for the four-region world.
// A region only becomes selectable after its campaign entry point is validated.
static const struct PgrRegionDefinition sWorldRegions[PGR_WORLD_REGION_COUNT] =
{
    {
        .id = PGW_START_KANTO,
        .name = sText_Kanto,
        .availability = PGR_REGION_MAP_DATA,
        .intro = PGR_REGION_INTRO_NONE,
        .entryPoint = { -1, -1, WARP_ID_NONE, -1, -1 },
    },
    {
        .id = PGW_START_JOHTO,
        .name = sText_Johto,
        .availability = PGR_REGION_PLANNED,
        .intro = PGR_REGION_INTRO_NONE,
        .entryPoint = { -1, -1, WARP_ID_NONE, -1, -1 },
    },
    {
        .id = PGW_START_HOENN,
        .name = sText_Hoenn,
        .availability = PGR_REGION_PLAYABLE,
        .intro = PGR_REGION_INTRO_HOENN,
        .entryPoint =
        {
            MAP_GROUP(MAP_INSIDE_OF_TRUCK),
            MAP_NUM(MAP_INSIDE_OF_TRUCK),
            WARP_ID_NONE,
            -1,
            -1,
        },
    },
    {
        .id = PGW_START_SINNOH,
        .name = sText_Sinnoh,
        .availability = PGR_REGION_PLANNED,
        .intro = PGR_REGION_INTRO_NONE,
        .entryPoint = { -1, -1, WARP_ID_NONE, -1, -1 },
    },
};

STATIC_ASSERT(ARRAY_COUNT(sWorldRegions) == PGR_WORLD_REGION_COUNT, WorldRegionRegistryCountMismatch);

const struct PgrRegionDefinition *Pgr_GetWorldRegion(u32 index)
{
    if (index >= ARRAY_COUNT(sWorldRegions))
        return NULL;
    return &sWorldRegions[index];
}

const struct PgrRegionDefinition *Pgr_FindWorldRegion(enum PgwStartingRegion region)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sWorldRegions); i++)
    {
        if (sWorldRegions[i].id == region)
            return &sWorldRegions[i];
    }
    return NULL;
}

bool32 Pgr_IsWorldRegion(enum PgwStartingRegion region)
{
    return Pgr_FindWorldRegion(region) != NULL;
}

bool32 Pgr_CanStartAdventureInRegion(enum PgwStartingRegion region)
{
    const struct PgrRegionDefinition *definition = Pgr_FindWorldRegion(region);

    return definition != NULL
        && definition->availability == PGR_REGION_PLAYABLE
        && definition->intro != PGR_REGION_INTRO_NONE
        && definition->entryPoint.mapGroup >= 0
        && definition->entryPoint.mapNum >= 0;
}

const struct PgrRegionEntryPoint *Pgr_GetRegionEntryPoint(enum PgwStartingRegion region)
{
    const struct PgrRegionDefinition *definition = Pgr_FindWorldRegion(region);

    if (!Pgr_CanStartAdventureInRegion(region))
        return NULL;
    return &definition->entryPoint;
}
