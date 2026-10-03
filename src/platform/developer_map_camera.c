#ifdef PLATFORM_SDL2

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "global.h"
#include "decompress.h"
#include "fieldmap.h"
#include "overworld.h"
#include "resource_pack.h"
#include "tileset_resources.h"
#include "platform/developer_map_camera.h"
#include "constants/map_types.h"
#include "constants/map_groups.h"
#include "../data/map_group_count.h"

#define DEVELOPER_MAP_CAMERA_MAX_MAPS 512
#define DEVELOPER_MAP_CAMERA_TEXTURE_CACHE 48
#define DEVELOPER_MAP_CAMERA_MIN_ZOOM 0.002
#define DEVELOPER_MAP_CAMERA_MAX_ZOOM 8.0

struct DeveloperMapNode
{
    u8 mapGroup;
    u8 mapNum;
    s32 x;
    s32 y;
    const struct MapHeader *header;
    const struct MapLayout *layout;
    SDL_Texture *texture;
    u32 textureUse;
    bool positioned;
    int component;
};

struct DeveloperMapCamera
{
    bool open;
    bool dragging;
    int dragX;
    int dragY;
    double centerX;
    double centerY;
    double zoom;
    u32 textureClock;
    int mapCount;
    int currentMap;
    int componentCount;
    s32 minimumX;
    s32 minimumY;
    s32 maximumX;
    s32 maximumY;
    bool needsFit;
    bool viewDirty;
    SDL_Texture *viewTexture;
    int viewWidth;
    int viewHeight;
    struct DeveloperMapNode maps[DEVELOPER_MAP_CAMERA_MAX_MAPS];
};

static struct DeveloperMapCamera sCamera;
extern const struct MapHeader *const *const gMapGroups[];

static int FindMap(u8 mapGroup, u8 mapNum)
{
    int i;

    for (i = 0; i < sCamera.mapCount; i++)
    {
        if (sCamera.maps[i].mapGroup == mapGroup && sCamera.maps[i].mapNum == mapNum)
            return i;
    }
    return -1;
}

static bool AddMap(u8 mapGroup, u8 mapNum)
{
    struct DeveloperMapNode *node;
    const struct MapHeader *header;
    const struct MapLayout *layout;

    if (sCamera.mapCount >= DEVELOPER_MAP_CAMERA_MAX_MAPS)
        return false;

    header = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    if (header == NULL || header->mapLayoutId == 0)
        return false;
    layout = GetMapLayout(header->mapLayoutId);
    if (layout == NULL)
        return false;

    node = &sCamera.maps[sCamera.mapCount++];
    memset(node, 0, sizeof(*node));
    node->mapGroup = mapGroup;
    node->mapNum = mapNum;
    node->component = -1;
    node->header = header;
    node->layout = layout;
    return true;
}

static bool IsExternalMapType(u8 mapType)
{
    return mapType == MAP_TYPE_TOWN
        || mapType == MAP_TYPE_CITY
        || mapType == MAP_TYPE_ROUTE
        || mapType == MAP_TYPE_OCEAN_ROUTE;
}

static void EnumerateExternalMaps(void)
{
    u16 mapGroup;

    for (mapGroup = 0; mapGroup < MAP_GROUPS_COUNT; mapGroup++)
    {
        u16 mapNum;
        if (gMapGroups[mapGroup] == NULL)
            continue;
        for (mapNum = 0; mapNum < MAP_GROUP_COUNT[mapGroup]; mapNum++)
        {
            const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
            if (header != NULL && IsExternalMapType(header->mapType))
                AddMap((u8)mapGroup, (u8)mapNum);
        }
    }
}

static void GetConnectedPosition(const struct DeveloperMapNode *node,
                                 const struct DeveloperMapNode *connected,
                                 const struct MapConnection *connection,
                                 s32 *x, s32 *y)
{
    *x = node->x;
    *y = node->y;
    switch (connection->direction)
    {
    case CONNECTION_NORTH:
        *x += connection->offset;
        *y -= connected->layout->height;
        break;
    case CONNECTION_SOUTH:
        *x += connection->offset;
        *y += node->layout->height;
        break;
    case CONNECTION_WEST:
        *x -= connected->layout->width;
        *y += connection->offset;
        break;
    case CONNECTION_EAST:
        *x += node->layout->width;
        *y += connection->offset;
        break;
    }
}

static void PositionComponent(int seedIndex, int component, s32 *minimumX, s32 *minimumY,
                              s32 *maximumX, s32 *maximumY)
{
    int queue[DEVELOPER_MAP_CAMERA_MAX_MAPS];
    int readAt = 0;
    int writeAt = 0;

    sCamera.maps[seedIndex].positioned = true;
    sCamera.maps[seedIndex].component = component;
    sCamera.maps[seedIndex].x = 0;
    sCamera.maps[seedIndex].y = 0;
    queue[writeAt++] = seedIndex;

    while (readAt < writeAt)
    {
        struct DeveloperMapNode *node = &sCamera.maps[queue[readAt++]];
        const struct MapConnections *connections = node->header->connections;
        int connectionIndex;

        if (connections == NULL)
            continue;
        for (connectionIndex = 0; connectionIndex < connections->count; connectionIndex++)
        {
            const struct MapConnection *connection = &connections->connections[connectionIndex];
            int connectedIndex = FindMap(connection->mapGroup, connection->mapNum);
            struct DeveloperMapNode *connected;
            s32 x;
            s32 y;

            if (connectedIndex < 0)
                continue;
            connected = &sCamera.maps[connectedIndex];
            if (connected->positioned)
                continue;
            GetConnectedPosition(node, connected, connection, &x, &y);
            connected->x = x;
            connected->y = y;
            connected->positioned = true;
            connected->component = component;
            queue[writeAt++] = connectedIndex;
        }
    }

    *minimumX = *maximumX = sCamera.maps[seedIndex].x;
    *minimumY = *maximumY = sCamera.maps[seedIndex].y;
    for (readAt = 0; readAt < writeAt; readAt++)
    {
        const struct DeveloperMapNode *node = &sCamera.maps[queue[readAt]];
        if (node->x < *minimumX) *minimumX = node->x;
        if (node->y < *minimumY) *minimumY = node->y;
        if (node->x + node->layout->width > *maximumX) *maximumX = node->x + node->layout->width;
        if (node->y + node->layout->height > *maximumY) *maximumY = node->y + node->layout->height;
    }
}

static void TranslateComponent(int component, s32 deltaX, s32 deltaY)
{
    int i;
    for (i = 0; i < sCamera.mapCount; i++)
    {
        if (sCamera.maps[i].component == component)
        {
            sCamera.maps[i].x += deltaX;
            sCamera.maps[i].y += deltaY;
        }
    }
}

static void PlaceJohtoRelativeToKanto(void)
{
    int route22 = FindMap(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22));
    int route26North = FindMap(MAP_GROUP(MAP_ROUTE26NORTH_HNS), MAP_NUM(MAP_ROUTE26NORTH_HNS));
    int route34 = FindMap(MAP_GROUP(MAP_ROUTE34_HNS), MAP_NUM(MAP_ROUTE34_HNS));
    int ilex = FindMap(MAP_GROUP(MAP_ILEX_FOREST_HNS), MAP_NUM(MAP_ILEX_FOREST_HNS));
    int ecruteak = FindMap(MAP_GROUP(MAP_ECRUTEAK_CITY_HNS), MAP_NUM(MAP_ECRUTEAK_CITY_HNS));
    int route42 = FindMap(MAP_GROUP(MAP_ROUTE42_HNS), MAP_NUM(MAP_ROUTE42_HNS));
    int route45 = FindMap(MAP_GROUP(MAP_ROUTE45_HNS), MAP_NUM(MAP_ROUTE45_HNS));
    int route46 = FindMap(MAP_GROUP(MAP_ROUTE46_HNS), MAP_NUM(MAP_ROUTE46_HNS));
    int blackthorn = FindMap(MAP_GROUP(MAP_BLACKTHORN_CITY_HNS), MAP_NUM(MAP_BLACKTHORN_CITY_HNS));
    s32 targetX;
    s32 targetY;
    int i;

    if (route22 < 0 || route26North < 0)
        return;
    // The former direct edge gave Johto y = Route 22 y - 10. A thirty-tile
    // northern move makes that y - 40; a future passage must fill the gap.
    targetX = sCamera.maps[route22].x - sCamera.maps[route26North].layout->width;
    targetY = sCamera.maps[route22].y - 40;
    TranslateComponent(sCamera.maps[route26North].component,
                       targetX - sCamera.maps[route26North].x,
                       targetY - sCamera.maps[route26North].y);

    // Ilex is now entered through its two gates, not an outdoor map edge.
    if (route34 >= 0 && ilex >= 0
     && sCamera.maps[route34].component != sCamera.maps[ilex].component)
    {
        targetX = sCamera.maps[route34].x - 102;
        targetY = sCamera.maps[route34].y + 40;
        TranslateComponent(sCamera.maps[ilex].component,
                           targetX - sCamera.maps[ilex].x,
                           targetY - sCamera.maps[ilex].y);
    }

    // Keep the eastern Johto cluster coherent while reserving a real 37-tile
    // corridor between Ecruteak and Route 42. The old edge is not playable.
    if (ecruteak >= 0 && route42 >= 0
     && sCamera.maps[ecruteak].component != sCamera.maps[route42].component)
    {
        targetX = sCamera.maps[ecruteak].x + sCamera.maps[ecruteak].layout->width + 37;
        targetY = sCamera.maps[ecruteak].y + 27;
        TranslateComponent(sCamera.maps[route42].component,
                           targetX - sCamera.maps[route42].x,
                           targetY - sCamera.maps[route42].y);
    }

    // Provisional atlas alignment requested for comparing the Route 45/46
    // terrain. Keep Blackthorn directly above Route 45. This is visual only:
    // the playable Route 44/Blackthorn edge needs a wider geography revision.
    if (route45 >= 0 && route46 >= 0 && blackthorn >= 0)
    {
        s32 deltaX = sCamera.maps[route46].x + sCamera.maps[route46].layout->width
                     - sCamera.maps[route45].x;
        sCamera.maps[route45].x += deltaX;
        sCamera.maps[blackthorn].x += deltaX;
    }

    sCamera.minimumX = sCamera.minimumY = INT_MAX;
    sCamera.maximumX = sCamera.maximumY = INT_MIN;
    for (i = 0; i < sCamera.mapCount; i++)
    {
        struct DeveloperMapNode *node = &sCamera.maps[i];
        // Johto is one reserved map group. Move every outdoor component,
        // including detached safari and mountain areas, together on the atlas.
        if (node->mapGroup == MAP_GROUP(MAP_NEW_BARK_TOWN_HNS))
            node->y += 10;
        if (node->x < sCamera.minimumX) sCamera.minimumX = node->x;
        if (node->y < sCamera.minimumY) sCamera.minimumY = node->y;
        if (node->x + node->layout->width > sCamera.maximumX)
            sCamera.maximumX = node->x + node->layout->width;
        if (node->y + node->layout->height > sCamera.maximumY)
            sCamera.maximumY = node->y + node->layout->height;
    }
}

static void PositionExternalMaps(void)
{
    const s32 gap = 16;
    const s32 rowLimit = 600;
    s32 cursorX = 0;
    s32 cursorY = 0;
    s32 rowHeight = 0;
    int preferredSeed = sCamera.currentMap >= 0 ? sCamera.currentMap : 0;
    int pass;

    sCamera.minimumX = INT_MAX;
    sCamera.minimumY = INT_MAX;
    sCamera.maximumX = INT_MIN;
    sCamera.maximumY = INT_MIN;

    for (pass = -1; pass < sCamera.mapCount; pass++)
    {
        int seed = pass < 0 ? preferredSeed : pass;
        s32 minimumX;
        s32 minimumY;
        s32 maximumX;
        s32 maximumY;
        s32 width;
        s32 height;

        if (seed < 0 || seed >= sCamera.mapCount || sCamera.maps[seed].positioned)
            continue;
        PositionComponent(seed, sCamera.componentCount, &minimumX, &minimumY, &maximumX, &maximumY);
        width = maximumX - minimumX;
        height = maximumY - minimumY;
        if (cursorX != 0 && cursorX + width > rowLimit)
        {
            cursorX = 0;
            cursorY += rowHeight + gap;
            rowHeight = 0;
        }
        TranslateComponent(sCamera.componentCount, cursorX - minimumX, cursorY - minimumY);
        minimumX = cursorX;
        minimumY = cursorY;
        maximumX = cursorX + width;
        maximumY = cursorY + height;
        if (minimumX < sCamera.minimumX) sCamera.minimumX = minimumX;
        if (minimumY < sCamera.minimumY) sCamera.minimumY = minimumY;
        if (maximumX > sCamera.maximumX) sCamera.maximumX = maximumX;
        if (maximumY > sCamera.maximumY) sCamera.maximumY = maximumY;
        cursorX += width + gap;
        if (height > rowHeight) rowHeight = height;
        sCamera.componentCount++;
    }
    PlaceJohtoRelativeToKanto();
}

static bool DecompressTiles(const struct Tileset *tileset, u8 *output, size_t outputSize)
{
    const u32 *input = ResolveTilesetTiles(tileset->tiles);
    u64 resourceSize;

    // A camera nao deve interpretar o identificador compilado de 4 bytes como
    // dados de tiles quando o pacote externo estiver ausente ou desatualizado.
    if (input == NULL || !ResourcePack_GetCachedSize(input, &resourceSize))
        return false;
    if (!tileset->isCompressed)
    {
        memcpy(output, input, resourceSize < outputSize ? (size_t)resourceSize : outputSize);
        return true;
    }
    if (resourceSize < sizeof(u32) || GetDecompressedDataSize(input) == 0
     || GetDecompressedDataSize(input) > outputSize)
        return false;
    DecompressDataWithHeaderWram(input, output);
    return true;
}

static Uint32 ColorToRgba(u16 color)
{
    Uint32 red = (color & 0x1F) * 255 / 31;
    Uint32 green = ((color >> 5) & 0x1F) * 255 / 31;
    Uint32 blue = ((color >> 10) & 0x1F) * 255 / 31;
    return 0xFF000000 | (red << 16) | (green << 8) | blue;
}

static void DrawTile(Uint32 *pixels, int imageWidth, int destinationX, int destinationY,
                     const u8 *tiles, const Uint32 *palette, u16 tileReference, bool transparent)
{
    u16 tileId = tileReference & 0x03FF;
    bool horizontalFlip = (tileReference & 0x0400) != 0;
    bool verticalFlip = (tileReference & 0x0800) != 0;
    u8 paletteId = tileReference >> 12;
    const u8 *tile;
    int y;

    if (tileId >= NUM_TILES_TOTAL || paletteId >= NUM_PALS_TOTAL)
        return;
    tile = tiles + tileId * 32;

    for (y = 0; y < 8; y++)
    {
        int sourceY = verticalFlip ? 7 - y : y;
        int x;
        for (x = 0; x < 8; x++)
        {
            int sourceX = horizontalFlip ? 7 - x : x;
            u8 packed = tile[sourceY * 4 + sourceX / 2];
            u8 colorId = sourceX & 1 ? packed >> 4 : packed & 0x0F;
            if (!transparent || colorId != 0)
                pixels[(destinationY + y) * imageWidth + destinationX + x] = palette[paletteId * 16 + colorId];
        }
    }
}

static bool GetMetatile(const struct MapLayout *layout, u16 mapEntry, const u16 **metatileOut)
{
    u16 metatileId = mapEntry & MAPGRID_METATILE_ID_MASK;
    u32 primaryCount = GetNumMetatilesInPrimary(layout);
    const u16 *metatiles;
    u64 resourceSize;
    u32 availableCount = NUM_METATILES_TOTAL;

    if (metatileId < primaryCount)
        metatiles = ResolveTilesetMetatiles(layout->primaryTileset->metatiles);
    else
    {
        metatiles = ResolveTilesetMetatiles(layout->secondaryTileset->metatiles);
        metatileId -= primaryCount;
    }
    if (metatiles == NULL)
        return false;
    if (ResourcePack_GetCachedSize(metatiles, &resourceSize))
        availableCount = (u32)(resourceSize / (sizeof(u16) * NUM_TILES_PER_METATILE));
    if (metatileId >= availableCount)
        return false;
    *metatileOut = metatiles + metatileId * NUM_TILES_PER_METATILE;
    return true;
}

static SDL_Texture *CreateMapTexture(SDL_Renderer *renderer, const struct MapLayout *layout)
{
    u32 primaryTiles = GetNumTilesInPrimary(layout);
    u32 primaryPalettes = GetNumPalsInPrimary(layout);
    size_t tileBytes = NUM_TILES_TOTAL * 32;
    int imageWidth = layout->width * 16;
    int imageHeight = layout->height * 16;
    u8 *tiles;
    Uint32 palette[NUM_PALS_TOTAL * 16];
    Uint32 *pixels;
    SDL_Texture *texture;
    const u16 (*primaryPalette)[16];
    const u16 (*secondaryPalette)[16];
    int x;
    int y;
    u32 i;

    if (layout == NULL
     || layout->primaryTileset == NULL
     || layout->secondaryTileset == NULL
     || layout->map == NULL
     || primaryTiles > NUM_TILES_TOTAL
     || primaryPalettes > NUM_PALS_TOTAL
     || layout->width <= 0 || layout->height <= 0
     || imageWidth > 4096 || imageHeight > 4096)
        return NULL;

    tiles = calloc(1, tileBytes);
    pixels = malloc((size_t)imageWidth * imageHeight * sizeof(*pixels));
    if (tiles == NULL || pixels == NULL)
    {
        free(tiles);
        free(pixels);
        return NULL;
    }

    if (!DecompressTiles(layout->primaryTileset, tiles, primaryTiles * 32)
     || !DecompressTiles(layout->secondaryTileset, tiles + primaryTiles * 32, (NUM_TILES_TOTAL - primaryTiles) * 32))
    {
        free(tiles);
        free(pixels);
        return NULL;
    }

    primaryPalette = ResolveTilesetPalettes(layout->primaryTileset->palettes);
    secondaryPalette = ResolveTilesetPalettes(layout->secondaryTileset->palettes);
    if (primaryPalette == NULL || secondaryPalette == NULL)
    {
        free(tiles);
        free(pixels);
        return NULL;
    }
    for (i = 0; i < primaryPalettes * 16; i++)
        palette[i] = ColorToRgba(((const u16 *)primaryPalette)[i]);
    for (; i < NUM_PALS_TOTAL * 16; i++)
        palette[i] = ColorToRgba(((const u16 *)secondaryPalette)[i]);

    for (i = 0; i < (u32)imageWidth * imageHeight; i++)
        pixels[i] = palette[0];

    for (y = 0; y < layout->height; y++)
    {
        for (x = 0; x < layout->width; x++)
        {
            const u16 *metatile;
            int layer;
            int tileIndex;

            if (!GetMetatile(layout, layout->map[y * layout->width + x], &metatile))
                continue;
            for (layer = 0; layer < 2; layer++)
            {
                for (tileIndex = 0; tileIndex < 4; tileIndex++)
                {
                    DrawTile(pixels, imageWidth,
                             x * 16 + (tileIndex & 1) * 8,
                             y * 16 + (tileIndex >> 1) * 8,
                             tiles, palette, metatile[layer * 4 + tileIndex], layer != 0);
                }
            }
        }
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, imageWidth, imageHeight);
    if (texture != NULL)
    {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
        SDL_UpdateTexture(texture, NULL, pixels, imageWidth * sizeof(*pixels));
    }
    free(tiles);
    free(pixels);
    return texture;
}

static void EvictOldTexture(void)
{
    int textureCount = 0;
    int oldest = -1;
    int i;

    for (i = 0; i < sCamera.mapCount; i++)
    {
        if (sCamera.maps[i].texture == NULL)
            continue;
        textureCount++;
        if (i != sCamera.currentMap
         && (oldest < 0 || sCamera.maps[i].textureUse < sCamera.maps[oldest].textureUse))
            oldest = i;
    }
    if (textureCount >= DEVELOPER_MAP_CAMERA_TEXTURE_CACHE && oldest >= 0)
    {
        SDL_DestroyTexture(sCamera.maps[oldest].texture);
        sCamera.maps[oldest].texture = NULL;
    }
}

static SDL_Texture *GetMapTexture(SDL_Renderer *renderer, int mapIndex)
{
    struct DeveloperMapNode *node = &sCamera.maps[mapIndex];

    node->textureUse = ++sCamera.textureClock;
    if (node->texture == NULL)
    {
        EvictOldTexture();
        node->texture = CreateMapTexture(renderer, node->layout);
    }
    return node->texture;
}

bool DeveloperMapCamera_Open(SDL_Renderer *renderer)
{
    u8 mapGroup;
    u8 mapNum;

    if (sCamera.open)
        return true;
    if (renderer == NULL || gSaveBlock1Ptr == NULL || gMapHeader.mapLayout == NULL)
        return false;

    memset(&sCamera, 0, sizeof(sCamera));
    mapGroup = gSaveBlock1Ptr->location.mapGroup;
    mapNum = gSaveBlock1Ptr->location.mapNum;
    DBGPRINTF("PC camera: opening from %u,%u\n", mapGroup, mapNum);
    EnumerateExternalMaps();
    DBGPRINTF("PC camera: enumerated %d outdoor maps\n", sCamera.mapCount);
    if (sCamera.mapCount == 0)
        return false;
    sCamera.currentMap = FindMap(mapGroup, mapNum);
    PositionExternalMaps();
    DBGPRINTF("PC camera: positioned %d components\n", sCamera.componentCount);
    sCamera.zoom = 1.0;
    sCamera.needsFit = true;
    sCamera.viewDirty = true;
    sCamera.open = true;
    return true;
}

void DeveloperMapCamera_Close(void)
{
    int i;

    for (i = 0; i < sCamera.mapCount; i++)
    {
        if (sCamera.maps[i].texture != NULL)
            SDL_DestroyTexture(sCamera.maps[i].texture);
    }
    if (sCamera.viewTexture != NULL)
        SDL_DestroyTexture(sCamera.viewTexture);
    memset(&sCamera, 0, sizeof(sCamera));
}

bool DeveloperMapCamera_IsOpen(void)
{
    return sCamera.open;
}

static bool WindowToRenderer(SDL_Window *window, SDL_Renderer *renderer, int windowX, int windowY, int *rendererX, int *rendererY)
{
    int windowWidth;
    int windowHeight;
    int rendererWidth;
    int rendererHeight;

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    SDL_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);
    if (windowWidth <= 0 || windowHeight <= 0)
        return false;
    *rendererX = windowX * rendererWidth / windowWidth;
    *rendererY = windowY * rendererHeight / windowHeight;
    return true;
}

bool DeveloperMapCamera_HandleEvent(const SDL_Event *event, SDL_Window *window, SDL_Renderer *renderer, const SDL_Rect *viewport)
{
    int x;
    int y;

    if (!sCamera.open)
        return false;

    switch (event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button != SDL_BUTTON_LEFT
         || !WindowToRenderer(window, renderer, event->button.x, event->button.y, &x, &y)
         || x < viewport->x || y < viewport->y || x >= viewport->x + viewport->w || y >= viewport->y + viewport->h)
            return true;
        sCamera.dragging = true;
        sCamera.dragX = x;
        sCamera.dragY = y;
        return true;
    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT)
            sCamera.dragging = false;
        return true;
    case SDL_MOUSEMOTION:
        if (!sCamera.dragging
         || !WindowToRenderer(window, renderer, event->motion.x, event->motion.y, &x, &y))
            return true;
        sCamera.centerX -= (x - sCamera.dragX) / sCamera.zoom;
        sCamera.centerY -= (y - sCamera.dragY) / sCamera.zoom;
        sCamera.dragX = x;
        sCamera.dragY = y;
        sCamera.viewDirty = true;
        return true;
    case SDL_MOUSEWHEEL:
    {
        int mouseWindowX;
        int mouseWindowY;
        double oldZoom = sCamera.zoom;
        double worldX;
        double worldY;

        SDL_GetMouseState(&mouseWindowX, &mouseWindowY);
        if (!WindowToRenderer(window, renderer, mouseWindowX, mouseWindowY, &x, &y))
            return true;
        worldX = sCamera.centerX + (x - (viewport->x + viewport->w / 2.0)) / oldZoom;
        worldY = sCamera.centerY + (y - (viewport->y + viewport->h / 2.0)) / oldZoom;
        if (event->wheel.y > 0)
            sCamera.zoom *= pow(1.25, event->wheel.y);
        else if (event->wheel.y < 0)
            sCamera.zoom /= pow(1.25, -event->wheel.y);
        if (sCamera.zoom < DEVELOPER_MAP_CAMERA_MIN_ZOOM)
            sCamera.zoom = DEVELOPER_MAP_CAMERA_MIN_ZOOM;
        if (sCamera.zoom > DEVELOPER_MAP_CAMERA_MAX_ZOOM)
            sCamera.zoom = DEVELOPER_MAP_CAMERA_MAX_ZOOM;
        sCamera.centerX = worldX - (x - (viewport->x + viewport->w / 2.0)) / sCamera.zoom;
        sCamera.centerY = worldY - (y - (viewport->y + viewport->h / 2.0)) / sCamera.zoom;
        sCamera.viewDirty = true;
        DeveloperMapCamera_UpdateWindowTitle(window);
        return true;
    }
    default:
        return false;
    }
}

static void FitAllExternalMaps(const SDL_Rect *viewport)
{
    double worldWidth = (sCamera.maximumX - sCamera.minimumX) * 16.0;
    double worldHeight = (sCamera.maximumY - sCamera.minimumY) * 16.0;
    double horizontalZoom;
    double verticalZoom;

    if (worldWidth <= 0.0 || worldHeight <= 0.0)
        return;
    horizontalZoom = viewport->w * 0.94 / worldWidth;
    verticalZoom = viewport->h * 0.94 / worldHeight;
    sCamera.zoom = horizontalZoom < verticalZoom ? horizontalZoom : verticalZoom;
    if (sCamera.zoom < DEVELOPER_MAP_CAMERA_MIN_ZOOM)
        sCamera.zoom = DEVELOPER_MAP_CAMERA_MIN_ZOOM;
    if (sCamera.zoom > 1.0)
        sCamera.zoom = 1.0;
    sCamera.centerX = (sCamera.minimumX + sCamera.maximumX) * 8.0;
    sCamera.centerY = (sCamera.minimumY + sCamera.maximumY) * 8.0;
    sCamera.needsFit = false;
    sCamera.viewDirty = true;
}

static void DrawMapView(SDL_Renderer *renderer, const SDL_Rect *viewport)
{
    int i;

    SDL_SetRenderDrawColor(renderer, 8, 12, 18, 255);
    SDL_RenderFillRect(renderer, viewport);

    for (i = 0; i < sCamera.mapCount; i++)
    {
        struct DeveloperMapNode *node = &sCamera.maps[i];
        SDL_Rect destination = {
            viewport->x + viewport->w / 2 + (int)lround((node->x * 16.0 - sCamera.centerX) * sCamera.zoom),
            viewport->y + viewport->h / 2 + (int)lround((node->y * 16.0 - sCamera.centerY) * sCamera.zoom),
            (int)ceil(node->layout->width * 16.0 * sCamera.zoom),
            (int)ceil(node->layout->height * 16.0 * sCamera.zoom)
        };
        SDL_Texture *texture;

        if (destination.x >= viewport->x + viewport->w || destination.y >= viewport->y + viewport->h
         || destination.x + destination.w <= viewport->x || destination.y + destination.h <= viewport->y)
            continue;
        texture = GetMapTexture(renderer, i);
        if (texture != NULL)
            SDL_RenderCopy(renderer, texture, NULL, &destination);
    }

    if (sCamera.currentMap >= 0 && sCamera.currentMap < sCamera.mapCount)
    {
        struct DeveloperMapNode *node = &sCamera.maps[sCamera.currentMap];
        SDL_Rect player = {
            viewport->x + viewport->w / 2 + (int)lround(((node->x + gSaveBlock1Ptr->pos.x) * 16.0 - sCamera.centerX) * sCamera.zoom),
            viewport->y + viewport->h / 2 + (int)lround(((node->y + gSaveBlock1Ptr->pos.y) * 16.0 - sCamera.centerY) * sCamera.zoom),
            (int)ceil(16.0 * sCamera.zoom),
            (int)ceil(16.0 * sCamera.zoom)
        };
        if (player.w < 4) player.w = 4;
        if (player.h < 4) player.h = 4;
        SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);
        SDL_RenderDrawRect(renderer, &player);
    }
}

void DeveloperMapCamera_Draw(SDL_Renderer *renderer, const SDL_Rect *viewport)
{
    SDL_Texture *previousTarget;
    SDL_Rect localViewport;

    if (sCamera.needsFit)
        FitAllExternalMaps(viewport);
    if (sCamera.viewTexture == NULL || sCamera.viewWidth != viewport->w || sCamera.viewHeight != viewport->h)
    {
        if (sCamera.viewTexture != NULL)
            SDL_DestroyTexture(sCamera.viewTexture);
        sCamera.viewTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_TARGET, viewport->w, viewport->h);
        sCamera.viewWidth = viewport->w;
        sCamera.viewHeight = viewport->h;
        sCamera.viewDirty = true;
    }

    if (sCamera.viewTexture == NULL)
    {
        DrawMapView(renderer, viewport);
        return;
    }
    if (sCamera.viewDirty)
    {
        previousTarget = SDL_GetRenderTarget(renderer);
        if (SDL_SetRenderTarget(renderer, sCamera.viewTexture) != 0)
        {
            DrawMapView(renderer, viewport);
            return;
        }
        localViewport.x = 0;
        localViewport.y = 0;
        localViewport.w = viewport->w;
        localViewport.h = viewport->h;
        DrawMapView(renderer, &localViewport);
        SDL_SetRenderTarget(renderer, previousTarget);
        sCamera.viewDirty = false;
    }
    SDL_RenderCopy(renderer, sCamera.viewTexture, NULL, viewport);
}

void DeveloperMapCamera_UpdateWindowTitle(SDL_Window *window)
{
    // Until the canonical ORAS glyph sheet is converted, do not introduce a
    // second provisional font into the game interface. Controls are documented
    // externally and the existing application title remains unchanged.
    SDL_SetWindowTitle(window, "Pokemon Regionalidades");
}

#endif // PLATFORM_SDL2
