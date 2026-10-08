/* Memory-only characterization of pinned reference functions.
 * BagCursor is the real upstream header; PartyMenu is a minimal collaborator.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define POKEHEARTGOLD_HEAP_H
enum HeapID { HEAP_TEST };
#include "bag_cursor.h"
typedef struct { int topScreenPanelShow, topScreenPanelYPos; } PartyMenu;
static int displacement;
static void PartyMenu_SetTopScreenSelectionPanelYDisplacement(PartyMenu *p, int y) { displacement = y; }

/* PRODUCTION_FUNCTIONS */

#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(void) {
    BagCursor cursor = {0};
    u8 position, scroll;
    for (u32 i = 0; i < 8; i++) BagCursor_Field_PocketSetPosition(&cursor, i, i + 1, i + 10);
    for (u32 i = 0; i < 5; i++) BagCursor_Battle_PocketSetPosition(&cursor, i, i + 20, i + 30);
    BagCursor_Field_SetPocket(&cursor, 7);
    BagCursor_Battle_SetPocket(&cursor, 4);
    BagCursor_Battle_SetLastUsedItem(&cursor, 25, 3);
    for (u32 i = 0; i < 8; i++) {
        BagCursor_Field_PocketGetPosition(&cursor, i, &position, &scroll);
        CHECK(position == i + 1 && scroll == i + 10);
    }
    for (u32 i = 0; i < 5; i++) {
        BagCursor_Battle_PocketGetPosition(&cursor, i, &position, &scroll);
        CHECK(position == i + 20 && scroll == i + 30);
    }
    CHECK(BagCursor_Field_GetPocket(&cursor) == 7 && BagCursor_Battle_GetPocket(&cursor) == 4);
    CHECK(BagCursor_Battle_GetLastUsedItem(&cursor) == 25 && BagCursor_Battle_GetLastUsedPocket(&cursor) == 3);
    BagCursorField field = cursor.field;
    BagCursor_Battle_Init(&cursor);
    CHECK(memcmp(&field, &cursor.field, sizeof(field)) == 0);
    CHECK(BagCursor_Battle_GetPocket(&cursor) == 0);
    // The reference reset does not erase the last-used item metadata.
    CHECK(BagCursor_Battle_GetLastUsedItem(&cursor) == 25 && BagCursor_Battle_GetLastUsedPocket(&cursor) == 3);
    for (u32 i = 0; i < 5; i++) {
        BagCursor_Battle_PocketGetPosition(&cursor, i, &position, &scroll);
        CHECK(position == 0 && scroll == 0);
    }
    PartyMenu party = { .topScreenPanelShow = 1 };
    const int opening[] = {12, 24, 36, 40, 40};
    for (int i = 0; i < 5; i++) {
        PartyMenu_UpdateTopScreenPanelYCoordFrame(&party);
        CHECK(party.topScreenPanelYPos == opening[i] && displacement == opening[i]);
    }
    party.topScreenPanelShow = 0;
    const int closing[] = {28, 16, 4, 0, 0};
    for (int i = 0; i < 5; i++) {
        PartyMenu_UpdateTopScreenPanelYCoordFrame(&party);
        CHECK(party.topScreenPanelYPos == closing[i] && displacement == closing[i]);
    }
    party.topScreenPanelShow = 1;
    PartyMenu_UpdateTopScreenPanelYCoordFrame(&party);
    party.topScreenPanelShow = 0;
    PartyMenu_UpdateTopScreenPanelYCoordFrame(&party);
    CHECK(displacement == 0);
    puts("HGSS application reference: field/battle cursors isolated; reset scope and panel transitions passed.");
    return 0;
}
