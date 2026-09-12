#ifndef GUARD_POKEMON_REGIONALIDADES_INVENTORY_H
#define GUARD_POKEMON_REGIONALIDADES_INVENTORY_H

bool32 PgrInventory_OnSaveLoaded(void);
bool32 PgrInventory_StageNativeSnapshot(void);
#ifdef PORTABLE
void PgrInventory_SyncFromLegacy(void);
void PgrInventory_BeginTemporaryLegacyOverride(void);
void PgrInventory_EndTemporaryLegacyOverride(void);
void PgrInventory_NotifyLegacySlotChanged(u32 location, u32 slot, u32 itemId, u32 quantity);
bool32 PgrInventory_TryGetLiveSlot(u32 location, u32 slot, u32 *itemId, u32 *quantity);
bool32 PgrInventory_TrySetLiveSlot(u32 location, u32 slot, u32 itemId, u32 quantity);
#else
#define PgrInventory_SyncFromLegacy() ((void)0)
#define PgrInventory_BeginTemporaryLegacyOverride() ((void)0)
#define PgrInventory_EndTemporaryLegacyOverride() ((void)0)
#define PgrInventory_NotifyLegacySlotChanged(location, slot, itemId, quantity) ((void)0)
#define PgrInventory_TryGetLiveSlot(location, slot, itemId, quantity) FALSE
#define PgrInventory_TrySetLiveSlot(location, slot, itemId, quantity) FALSE
#endif

#endif // GUARD_POKEMON_REGIONALIDADES_INVENTORY_H
