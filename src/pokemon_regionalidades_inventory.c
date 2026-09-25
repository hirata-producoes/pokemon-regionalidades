#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "pokemon_regionalidades_inventory.h"
#include "pokemon_storage_system.h"
#include "constants/flags.h"
#include "constants/items.h"
#ifdef PORTABLE
#include "platform/pc_inventory_state.h"
#include "platform/pc_save_container.h"
#endif

#ifdef PORTABLE
static struct PcInventoryState sNativeInventorySnapshot;
static struct PcInventoryState sLoadedInventorySnapshot;
static struct PcInventoryState sTemporaryInventorySnapshot;
static unsigned char sNativeInventoryEncoded[PC_INVENTORY_MAX_ENCODED_SIZE];
static bool32 sNativeInventoryReady;
static u32 sTemporaryInventoryDepth;

static u32 GetLegacyInventoryCapacity(u32 location)
{
    switch (location)
    {
    case POCKET_ITEMS:      return BAG_ITEMS_COUNT;
    case POCKET_KEY_ITEMS:  return BAG_KEYITEMS_COUNT;
    case POCKET_POKE_BALLS: return BAG_POKEBALLS_COUNT;
    case POCKET_TM_HM:      return BAG_TMHM_COUNT;
    case POCKET_BERRIES:    return BAG_BERRIES_COUNT;
    default:                return PC_ITEMS_COUNT;
    }
}

static bool32 RemoveLegacyHeldExpShares(void)
{
    bool32 found = FALSE;
    u16 noItem = ITEM_NONE;

    for (u32 slot = 0; slot < PARTY_SIZE; slot++)
    {
        if (GetMonData(&gSaveBlock1Ptr->playerParty[slot], MON_DATA_HELD_ITEM) == ITEM_EXP_SHARE)
        {
            SetMonData(&gSaveBlock1Ptr->playerParty[slot], MON_DATA_HELD_ITEM, &noItem);
            found = TRUE;
        }
    }

    if (gPokemonStoragePtr != NULL)
    {
        for (u32 box = 0; box < TOTAL_BOXES_COUNT; box++)
        {
            for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
            {
                struct BoxPokemon *mon = &gPokemonStoragePtr->boxes[box][slot];

                if (GetBoxMonData(mon, MON_DATA_HELD_ITEM) == ITEM_EXP_SHARE)
                {
                    SetBoxMonData(mon, MON_DATA_HELD_ITEM, &noItem);
                    found = TRUE;
                }
            }
        }
    }

    for (u32 slot = 0; slot < ARRAY_COUNT(gSaveBlock1Ptr->daycare.mons); slot++)
    {
        struct BoxPokemon *mon = &gSaveBlock1Ptr->daycare.mons[slot].mon;

        if (GetBoxMonData(mon, MON_DATA_HELD_ITEM) == ITEM_EXP_SHARE)
        {
            SetBoxMonData(mon, MON_DATA_HELD_ITEM, &noItem);
            found = TRUE;
        }
    }

    return found;
}

static void NormalizePermanentInventory(struct PcInventoryState *inventory, bool32 foundHeldExpShare)
{
    bool8 machines[NUM_ALL_MACHINES + 1] = {0};
    bool32 foundExpShare = foundHeldExpShare || FlagGet(FLAG_RECEIVED_EXP_SHARE);
    bool32 expShareWasKeyItem = FALSE;
    bool32 expShareInserted = FALSE;
    u32 expShareFallbackLocation = PC_INVENTORY_LOCATION_COUNT;
    u32 expShareFallbackSlot = PC_INVENTORY_MAX_SLOTS_PER_LOCATION;

    for (u32 location = 0; location < PC_INVENTORY_LOCATION_COUNT; location++)
    {
        for (u32 slot = 0; slot < PC_INVENTORY_MAX_SLOTS_PER_LOCATION; slot++)
        {
            struct PcInventorySlot *item = &inventory->slots[location][slot];
            enum TMHMIndex machine;

            if (item->itemId == ITEM_EXP_SHARE && item->quantity != 0)
            {
                foundExpShare = TRUE;
                expShareWasKeyItem |= location == POCKET_KEY_ITEMS;
                if (expShareFallbackLocation == PC_INVENTORY_LOCATION_COUNT)
                {
                    expShareFallbackLocation = location;
                    expShareFallbackSlot = slot;
                }
                item->itemId = ITEM_NONE;
                item->quantity = 0;
                continue;
            }

            if (item->itemId >= ITEMS_COUNT || item->quantity == 0)
                continue;
            machine = GetItemTMHMIndex(item->itemId);
            if (machine == 0 || GetItemImportance(item->itemId) != 1)
                continue;

            if (machines[machine])
            {
                item->itemId = ITEM_NONE;
                item->quantity = 0;
            }
            else
            {
                machines[machine] = TRUE;
                item->quantity = 1;
            }
        }
    }

    if (foundExpShare)
    {
        u32 locations[] = {POCKET_KEY_ITEMS, POCKETS_COUNT};

        for (u32 i = 0; i < ARRAY_COUNT(locations); i++)
        {
            u32 location = locations[i];
            u32 capacity = location < POCKETS_COUNT
                         ? gBagPockets[location].capacity
                         : PC_ITEMS_COUNT;

            for (u32 slot = 0; slot < capacity; slot++)
            {
                struct PcInventorySlot *item = &inventory->slots[location][slot];

                if (item->itemId == ITEM_NONE || item->quantity == 0)
                {
                    item->itemId = ITEM_EXP_SHARE;
                    item->quantity = 1;
                    expShareInserted = TRUE;
                    break;
                }
            }

            if (expShareInserted)
                break;
        }

        // Uma migração nunca pode apagar o item quando os destinos estão cheios.
        // Nesse caso raro, preserva-se a primeira posição original para tentar
        // novamente em uma versão futura, sem perda de dados do jogador.
        if (!expShareInserted
         && expShareFallbackLocation < PC_INVENTORY_LOCATION_COUNT
         && expShareFallbackSlot < PC_INVENTORY_MAX_SLOTS_PER_LOCATION)
        {
            inventory->slots[expShareFallbackLocation][expShareFallbackSlot].itemId = ITEM_EXP_SHARE;
            inventory->slots[expShareFallbackLocation][expShareFallbackSlot].quantity = 1;
        }

        // Saves antigos usavam o Exp. Share como item equipado e começam com
        // a distribuição moderna ligada. Depois disso, a escolha é preservada.
        if (!expShareWasKeyItem)
            FlagSet(FLAG_SYS_EXP_SHARE_ENABLED);
    }
}

static bool32 BuildLegacyInventory(struct PcInventoryState *inventory)
{
    memset(inventory, 0, sizeof(*inventory));
    inventory->authority = PC_INVENTORY_AUTHORITY_LEGACY_MIRROR;
    for (u32 pocket = 0; pocket < POCKETS_COUNT; pocket++)
    {
        if (gBagPockets[pocket].capacity > PC_INVENTORY_MAX_SLOTS_PER_LOCATION)
            return FALSE;
        for (u32 slot = 0; slot < GetLegacyInventoryCapacity(pocket); slot++)
        {
            const struct LegacyItemSlot *legacyItem = &gBagPockets[pocket].itemSlots[slot];
            u32 itemId = legacyItem->itemId;
            u32 quantity = (u16)(legacyItem->quantity ^ (u16)gSaveBlock2Ptr->encryptionKey);

            if (itemId == ITEM_NONE || quantity == 0)
                continue;
            inventory->slots[pocket][slot].itemId = itemId;
            inventory->slots[pocket][slot].quantity = quantity;
        }
    }

    for (u32 slot = 0; slot < PC_ITEMS_COUNT; slot++)
    {
        const struct LegacyItemSlot *item = &gSaveBlock1Ptr->pcItems[slot];

        if (item->itemId == ITEM_NONE || item->quantity == 0)
            continue;
        inventory->slots[POCKETS_COUNT][slot].itemId = item->itemId;
        inventory->slots[POCKETS_COUNT][slot].quantity = item->quantity;
    }
    return TRUE;
}

static bool32 CopyLegacyInventory(void)
{
    if (!BuildLegacyInventory(&sNativeInventorySnapshot))
        return FALSE;
    sNativeInventoryReady = TRUE;
    return TRUE;
}

static bool32 CanProjectNativeInventory(const struct PcInventoryState *inventory)
{
    for (u32 location = 0; location < PC_INVENTORY_LOCATION_COUNT; location++)
    {
        u32 legacyCapacity = location < POCKETS_COUNT
                           ? gBagPockets[location].capacity
                           : PC_ITEMS_COUNT;

        for (u32 slot = 0; slot < PC_INVENTORY_MAX_SLOTS_PER_LOCATION; slot++)
        {
            const struct PcInventorySlot *item = &inventory->slots[location][slot];

            if (item->itemId == ITEM_NONE && item->quantity == 0)
                continue;
            if (slot >= legacyCapacity
             || item->itemId == ITEM_NONE || item->itemId >= ITEMS_COUNT
             || item->quantity == 0 || item->quantity > PC_INVENTORY_MAX_QUANTITY)
                return FALSE;
        }
    }
    return TRUE;
}

static bool32 ProjectNativeInventoryToLegacy(const struct PcInventoryState *inventory)
{
    if (!CanProjectNativeInventory(inventory))
        return FALSE;

    for (u32 pocket = 0; pocket < POCKETS_COUNT; pocket++)
    {
        for (u32 slot = 0; slot < GetLegacyInventoryCapacity(pocket); slot++)
        {
            const struct PcInventorySlot *item = &inventory->slots[pocket][slot];
            struct LegacyItemSlot *legacyItem = &gBagPockets[pocket].itemSlots[slot];
            u32 legacyQuantity = min(item->quantity, MAX_LEGACY_ITEM_CAPACITY);

            legacyItem->itemId = item->itemId;
            legacyItem->quantity = (u16)legacyQuantity ^ (u16)gSaveBlock2Ptr->encryptionKey;
        }
    }
    for (u32 slot = 0; slot < PC_ITEMS_COUNT; slot++)
    {
        const struct PcInventorySlot *item = &inventory->slots[POCKETS_COUNT][slot];

        gSaveBlock1Ptr->pcItems[slot].itemId = item->itemId;
        gSaveBlock1Ptr->pcItems[slot].quantity = min(item->quantity, MAX_LEGACY_ITEM_CAPACITY);
    }
    memcpy(&sNativeInventorySnapshot, inventory, sizeof(sNativeInventorySnapshot));
    sNativeInventorySnapshot.authority = PC_INVENTORY_AUTHORITY_NATIVE;
    sNativeInventoryReady = TRUE;
    return TRUE;
}

static bool32 LegacyMirrorMatchesNative(void)
{
    if (!BuildLegacyInventory(&sLoadedInventorySnapshot))
        return FALSE;

    for (u32 location = 0; location < PC_INVENTORY_LOCATION_COUNT; location++)
    {
        u32 legacyCapacity = GetLegacyInventoryCapacity(location);

        for (u32 slot = 0; slot < PC_INVENTORY_MAX_SLOTS_PER_LOCATION; slot++)
        {
            const struct PcInventorySlot *nativeItem = &sNativeInventorySnapshot.slots[location][slot];
            const struct PcInventorySlot *legacyItem = &sLoadedInventorySnapshot.slots[location][slot];
            u32 expectedItemId = nativeItem->itemId;
            u32 expectedQuantity = min(nativeItem->quantity, MAX_LEGACY_ITEM_CAPACITY);

            // Posições nativas extras não existem no espelho Emerald.
            if (slot >= legacyCapacity)
                continue;
            if (legacyItem->itemId != expectedItemId
             || legacyItem->quantity != expectedQuantity)
                return FALSE;
        }
    }
    return TRUE;
}
#endif

#ifdef PORTABLE
void PgrInventory_SyncFromLegacy(void)
{
    // A full resync is reserved for the few legacy paths that replace or clear
    // whole arrays at once (new game, Wally's tutorial and link backups).
    if (!CopyLegacyInventory())
        sNativeInventoryReady = FALSE;
}

void PgrInventory_BeginTemporaryLegacyOverride(void)
{
    if (sTemporaryInventoryDepth != 0)
    {
        sTemporaryInventoryDepth++;
        return;
    }
    if (!sNativeInventoryReady && !CopyLegacyInventory())
        return;

    memcpy(&sTemporaryInventorySnapshot, &sNativeInventorySnapshot,
           sizeof(sTemporaryInventorySnapshot));
    sTemporaryInventoryDepth = 1;
}

void PgrInventory_EndTemporaryLegacyOverride(void)
{
    if (sTemporaryInventoryDepth == 0)
    {
        PgrInventory_SyncFromLegacy();
        return;
    }
    if (--sTemporaryInventoryDepth != 0)
        return;

    if (!ProjectNativeInventoryToLegacy(&sTemporaryInventorySnapshot))
        sNativeInventoryReady = FALSE;
}

void PgrInventory_NotifyLegacySlotChanged(u32 location, u32 slot, u32 itemId, u32 quantity)
{
    if (location >= PC_INVENTORY_LOCATION_COUNT
     || slot >= PC_INVENTORY_MAX_SLOTS_PER_LOCATION)
        return;
    if (!sNativeInventoryReady && !CopyLegacyInventory())
        return;

    if (itemId == ITEM_NONE || quantity == 0)
    {
        itemId = ITEM_NONE;
        quantity = 0;
    }
    else if (itemId >= ITEMS_COUNT || quantity > PC_INVENTORY_MAX_QUANTITY)
    {
        sNativeInventoryReady = FALSE;
        return;
    }
    sNativeInventorySnapshot.slots[location][slot].itemId = itemId;
    sNativeInventorySnapshot.slots[location][slot].quantity = quantity;
}

bool32 PgrInventory_TryGetLiveSlot(u32 location, u32 slot, u32 *itemId, u32 *quantity)
{
    const struct PcInventorySlot *nativeSlot;

    if (location >= PC_INVENTORY_LOCATION_COUNT
     || slot >= PC_INVENTORY_MAX_SLOTS_PER_LOCATION
     || itemId == NULL || quantity == NULL)
        return FALSE;
    if (!sNativeInventoryReady && !CopyLegacyInventory())
        return FALSE;

    nativeSlot = &sNativeInventorySnapshot.slots[location][slot];
    *itemId = nativeSlot->itemId;
    *quantity = nativeSlot->quantity;
    return TRUE;
}

bool32 PgrInventory_TrySetLiveSlot(u32 location, u32 slot, u32 itemId, u32 quantity)
{
    struct PcInventorySlot *nativeSlot;

    if (location >= PC_INVENTORY_LOCATION_COUNT
     || slot >= PC_INVENTORY_MAX_SLOTS_PER_LOCATION)
        return FALSE;
    if (!sNativeInventoryReady && !CopyLegacyInventory())
        return FALSE;

    if (itemId == ITEM_NONE || quantity == 0)
    {
        itemId = ITEM_NONE;
        quantity = 0;
    }
    else if (itemId >= ITEMS_COUNT || quantity > PC_INVENTORY_MAX_QUANTITY)
    {
        return FALSE;
    }

    nativeSlot = &sNativeInventorySnapshot.slots[location][slot];
    nativeSlot->itemId = itemId;
    nativeSlot->quantity = quantity;
    return TRUE;
}
#endif

bool32 PgrInventory_OnSaveLoaded(void)
{
#ifdef PORTABLE
    const unsigned char *data;
    size_t size;
    u32 schemaVersion;
    u32 flags;
    bool32 foundHeldExpShare;

    sNativeInventoryReady = FALSE;
    sTemporaryInventoryDepth = 0;
    foundHeldExpShare = RemoveLegacyHeldExpShares();

    if (!CopyLegacyInventory())
        return FALSE;
    if (!PcSaveContainer_GetChunk("INVENT", &data, &size, &schemaVersion, &flags))
    {
        NormalizePermanentInventory(&sNativeInventorySnapshot, foundHeldExpShare);
        if (!ProjectNativeInventoryToLegacy(&sNativeInventorySnapshot))
            return FALSE;
        sNativeInventorySnapshot.authority = PC_INVENTORY_AUTHORITY_NATIVE;
        return TRUE;
    }
    if (flags != PC_SAVE_CHUNK_REQUIRED
     || !PcInventoryState_Decode(data, size, schemaVersion, &sLoadedInventorySnapshot))
        return FALSE;

    if (sLoadedInventorySnapshot.authority == PC_INVENTORY_AUTHORITY_LEGACY_MIRROR)
    {
        if (memcmp(sLoadedInventorySnapshot.slots, sNativeInventorySnapshot.slots,
                   sizeof(sLoadedInventorySnapshot.slots)) != 0)
            return FALSE;
        NormalizePermanentInventory(&sNativeInventorySnapshot, foundHeldExpShare);
        if (!ProjectNativeInventoryToLegacy(&sNativeInventorySnapshot))
            return FALSE;
    }
    else
    {
        NormalizePermanentInventory(&sLoadedInventorySnapshot, foundHeldExpShare);
        if (ProjectNativeInventoryToLegacy(&sLoadedInventorySnapshot))
            return TRUE;

        // A newer inventory that no longer fits the current gameplay projection
        // must never be truncated into the legacy arrays.
        return FALSE;
    }
#endif
    return TRUE;
}

bool32 PgrInventory_StageNativeSnapshot(void)
{
#ifdef PORTABLE
    size_t encodedSize;

    if ((!sNativeInventoryReady && !CopyLegacyInventory())
     || !LegacyMirrorMatchesNative()
     || !CanProjectNativeInventory(&sNativeInventorySnapshot))
        return FALSE;
    sNativeInventorySnapshot.authority = PC_INVENTORY_AUTHORITY_NATIVE;
    if (!PcInventoryState_Encode(&sNativeInventorySnapshot, sNativeInventoryEncoded, &encodedSize))
        return FALSE;
    return PcSaveContainer_SetChunk("INVENT", PC_INVENTORY_STATE_SCHEMA_VERSION,
                                    PC_SAVE_CHUNK_REQUIRED,
                                    sNativeInventoryEncoded, encodedSize);
#else
    return TRUE;
#endif
}
