/* Reference-only trace harness. Minimal collaborators, NOT Nintendo struct ABI.
 * Upstream function bodies are inserted unchanged by the runner.
 * No framebuffer, SDK execution, game state or saves.
 */
#include <stdio.h>
#include <stdlib.h>
#include "constants/battle_menu.h"
#define MOVE_NONE 0
#define FALSE 0
#define TRUE 1
typedef int SysTask;
typedef int BOOL;
typedef int BgConfig;
typedef struct { int x, y; } TextObject;
typedef struct { TextObject *sprite; int y; } ManagedSprite;
typedef struct { int moveNo[4]; } BattleInputFightMenu;
typedef struct {
    void *battleSystem;
    void *feedbackTask, *unk10, *unused_664;
    int keyPressed;
    struct {
        int state, delay;
        union {
            struct {
                void *screenOffsets, *touchscreenHitbox;
                int ret, unk10, textObjId, pokemonIconIndex, shouldDeleteAfter;
            } button;
            struct { void *unk4, *unk8; } unkBugContest;
        };
    } feedback;
    struct { TextObject *textObj; } textObj[13];
    ManagedSprite *spritePokemonIcons[4], *spriteTypeIcons[4], *spriteCategoryIcons[4];
    struct { BattleInputFightMenu fight; } menu;
} BattleInput;
static int frame, deleted, cleaned;
static BgConfig *BattleSystem_GetBgConfig(void *system) { return NULL; }
static void ov12_02268EE4(BattleInput *b, void *offsets, void *hitbox, int buffer, int index) { frame = index; }
static void sub_02013794(TextObject *o, int *x, int *y) { *x = o->x; *y = o->y; }
static void sub_020136B4(TextObject *o, int x, int y) { o->x = x; o->y = y; }
static void ManagedSprite_OffsetPositionXY(ManagedSprite *o, int x, int y) { o->y += y; }
static void Sprite_OffsetPositionXY(TextObject *o, int x, int y) { o->x += x; o->y += y; }
static void ov12_02268D88(BattleInput *b, int a, int c) { cleaned++; }
static void BattleInput_DestroyFeedbackTask(BattleInput *b) { deleted++; }

/* PRODUCTION_FUNCTIONS */

#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(void) {
    TextObject texts[13] = {0}, badge = {0};
    ManagedSprite icon = {0}, type = { .sprite = &badge };
    BattleInput b = {0};
    for (int mask = 0; mask < 8; ++mask) {
        b.feedbackTask = mask & 1 ? &b : NULL;
        b.unk10 = mask & 2 ? &b : NULL;
        b.unused_664 = mask & 4 ? &b : NULL;
        CHECK(BattleInput_CheckFeedbackDone(&b) == (mask == 0));
    }
    b.feedbackTask = b.unk10 = b.unused_664 = NULL;
    CHECK(BattleInput_CheckFeedbackDone(&b));
    BattleInput_SetKeyPressed(&b, 1);
    CHECK(BattleInput_GetKeyPressed(&b) == 1);
    BattleInput_SetKeyPressed(&b, 0);
    CHECK(BattleInput_GetKeyPressed(&b) == 0);
    for (int i = 0; i < 13; i++) b.textObj[i].textObj = &texts[i];
    b.spritePokemonIcons[0] = &icon;
    Task_ButtonFeedback(NULL, &b);
    CHECK(frame == 2 && texts[0].y == -2 && icon.y == -2 && deleted == 0);
    Task_ButtonFeedback(NULL, &b);
    CHECK(frame == 1 && texts[0].y == -1 && icon.y == -1 && deleted == 0);
    Task_ButtonFeedback(NULL, &b);
    CHECK(deleted == 1 && cleaned == 1);
    b.feedback.state = b.feedback.delay = deleted = cleaned = 0;
    b.feedback.button.ret = BATTLE_INPUT_MOVE_1;
    b.menu.fight.moveNo[0] = 1;
    b.spriteTypeIcons[0] = &type;
    texts[0].y = 0;
    Task_FightMenuButtonFeedback(NULL, &b);
    CHECK(frame == 2 && texts[0].y == -2 && texts[MENUTXT_PP_1].y == -2);
    CHECK(texts[MENUTXT_PP_MAX_1].y == -2 && badge.y == -2 && deleted == 0);
    Task_FightMenuButtonFeedback(NULL, &b);
    CHECK(frame == 1 && texts[0].y == -1 && texts[MENUTXT_PP_1].y == -1 && badge.y == -1);
    Task_FightMenuButtonFeedback(NULL, &b);
    CHECK(frame == 0 && deleted == 1 && cleaned == 1);
    b.feedback.state = b.feedback.delay = deleted = cleaned = 0;
    b.feedback.button.ret = BATTLE_INPUT_CANCEL;
    Task_FightMenuButtonFeedback(NULL, &b);
    CHECK(texts[MENUTXT_CANCEL].y == -2 && badge.y == -1);
    Task_FightMenuButtonFeedback(NULL, &b);
    Task_FightMenuButtonFeedback(NULL, &b);
    CHECK(frame == 0 && texts[MENUTXT_CANCEL].y == -1 && deleted == 1);
    puts("HGSS reference: eight readiness combinations, input-origin getters and button/move/cancel traces passed (no rendering).");
    return 0;
}
