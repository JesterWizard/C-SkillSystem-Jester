#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "action-expa.h"
#include "icon-rework.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_SkillSwap) && (COMMON_SKILL_VALID(SID_SkillSwap))

#define SKILL_SLOT_COUNT UNIT_RAM_SKILLS_LEN
#define SKILL_ICON_PAL TILEREF(0, STATSCREEN_BGPAL_ITEMICONS)

// Define positions and sizes for the two menus.
#define leftX 1
#define leftY 3
#define menuWidth 14
#define menuHeight (SKILL_SLOT_COUNT + 1) * 2
#define rightX 15
#define rightY 3

// Helper: Check if a given skill slot has a valid skill.
static bool IsValidSkillSlot(struct Unit *unit, int slot) {
    int sid = GET_SKILL(unit, slot);
    return EQUIP_SKILL_VALID(sid);
}

// Helper: Check if any skill slot in the unit is valid.
static bool HasAnySkill(struct Unit *unit) {
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (IsValidSkillSlot(unit, i))
            return true;
    }
    return false;
}

// A helper to draw a skill entry – it draws the skill name and its icon.
static void DrawSkillSwapEntry(u16 *tilemap, int xTile, int yTile,
                               struct Unit *unit, int slot, u16 palette, struct Text *text)
{
    int sid = GET_SKILL(unit, slot);

    // Do not clear here – it’s already cleared in the update routine.
    Text_SetParams(text, 0, TEXT_COLOR_SYSTEM_GOLD);
    if (EQUIP_SKILL_VALID(sid)) {
        Text_DrawString(text, GetSkillNameStr(sid));
        DrawIcon(TILEMAP_LOCATED(tilemap, xTile, yTile), SKILL_ICON(sid), palette);
    }
    // Otherwise, leave the text blank.

    // Commit the text into the tilemap at an offset.
    PutText(text, TILEMAP_LOCATED(tilemap, xTile + 2, yTile));
}

// This proc structure stores both units and selection data.
// Proc structure now holds additional fields for a two‐step selection.
struct SkillSwapTradeMenuProc {
    PROC_HEADER;
    struct Unit *leftUnit;   // active unit
    struct Unit *rightUnit;  // target unit
    int leftSelected;        // current slot in left menu
    int rightSelected;       // current slot in right menu
    int activeSide;          // 0 = left active, 1 = right active
    int state;               // 0 = no skill selected; 1 = waiting for target selection
    int selectedColumn;      // the column (0 or 1) from which the skill was first selected
    int selectedRow;         // the row index from the selected side
    // Persistent text buffers for each skill entry:
    struct Text leftText[SKILL_SLOT_COUNT];
    struct Text rightText[SKILL_SLOT_COUNT];
};

// A simple implementation that draws a rectangular border using the given color.
static void DrawUiBox(u16 *tilemap, int x, int y, int width, int height, int color)
{
    int i;
    // Draw top and bottom borders.
    for (i = 0; i < width; i++) {
        tilemap[TILEMAP_INDEX(x + i, y)] = color;
        tilemap[TILEMAP_INDEX(x + i, y + height - 1)] = color;
    }
    // Draw left and right borders.
    for (i = 1; i < height - 1; i++) {
        tilemap[TILEMAP_INDEX(x, y + i)] = color;
        tilemap[TILEMAP_INDEX(x + width - 1, y + i)] = color;
    }
}

static void SkillSwapTradeMenu_Update(struct SkillSwapTradeMenuProc * proc)
{
    // Clear the BG (adjust BG0 as needed).
    BG_Fill(gBG0TilemapBuffer, 0);

    // Draw outlines (using a UI frame-drawing helper; if not available, you can draw rectangles manually).
    DrawUiFrame2(leftX, leftY, menuWidth, menuHeight, 0);
    DrawUiFrame2(rightX, rightY, menuWidth, menuHeight, 0);

    // Draw left unit's skill list.
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int drawX = leftX + 1;
        int drawY = leftY + 1 + (i * 2);
        DrawSkillSwapEntry(gBG0TilemapBuffer, drawX, drawY, proc->leftUnit, i, 0x4000, &proc->leftText[i]);
    }

    // Draw right unit's skill list.
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int drawX = rightX + 1;
        int drawY = rightY + 1 + (i * 2);
        DrawSkillSwapEntry(gBG0TilemapBuffer, drawX, drawY, proc->rightUnit, i, 0x4000, &proc->rightText[i]);
    }

    // Draw highlight boxes.
    if (proc->state == 0) {
        // Only the active menu shows a highlight.
        if (proc->activeSide == 0)
            DrawUiBox(gBG0TilemapBuffer, leftX, leftY + 1 + (proc->leftSelected * 2), menuWidth, 2, 0);
        else
            DrawUiBox(gBG0TilemapBuffer, rightX, rightY + 1 + (proc->rightSelected * 2), menuWidth, 2, 0);
    } else {
        // In state 1, show the frozen hand on the originally selected skill
        // and a regular highlight on the active menu.
        if (proc->selectedColumn == 0) {
            // Draw frozen highlight on left menu.
            DrawUiBox(gBG0TilemapBuffer, leftX, leftY + 1 + (proc->selectedRow * 2), menuWidth, 2, 1);
        } else {
            DrawUiBox(gBG0TilemapBuffer, rightX, rightY + 1 + (proc->selectedRow * 2), menuWidth, 2, 1);
        }
        // Then draw the active highlight on the current active menu.
        if (proc->activeSide == 0)
            DrawUiBox(gBG0TilemapBuffer, leftX, leftY + 1 + (proc->leftSelected * 2), menuWidth, 2, 0);
        else
            DrawUiBox(gBG0TilemapBuffer, rightX, rightY + 1 + (proc->rightSelected * 2), menuWidth, 2, 0);
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

// Helper to redraw a menu line for a given side and row.
static void RedrawSlot(int side, int row, struct SkillSwapTradeMenuProc *proc) {
    int x = (side == 0) ? (1 + leftX) : (15 + leftX);
    int y = leftY + 1 + row * 2;
    if (side == 0)
        DrawSkillSwapEntry(gBG0TilemapBuffer, x, y, proc->leftUnit, row, SKILL_ICON_PAL, &proc->leftText[row]);
    else
        DrawSkillSwapEntry(gBG0TilemapBuffer, x, y, proc->rightUnit, row, SKILL_ICON_PAL, &proc->rightText[row]);
}

// Helper to draw a UI hand (or frozen hand) at the correct position.
static void DrawHand(int side, int row, bool frozen) {
    int baseX = (side == 0) ? (1 + leftX) : (15 + leftX);
    int handX = baseX * 8;
    int handY = (leftY + 1 + row * 2) * 8;
    if (frozen)
        DisplayFrozenUiHand(handX, handY);
    else
        DisplayUiHand(handX, handY);
}

static void SkillSwapTradeMenu_OnLoop(struct SkillSwapTradeMenuProc *proc)
{
    u16 keys = gKeyStatusPtr->repeatedKeys;

    // Process directional input.
    // Process DPAD_UP: move to the previous valid skill.
    if (keys & DPAD_UP) {
        if (proc->activeSide == 0) {
            int newSel = proc->leftSelected;
            do { newSel--; } while (newSel >= 0 && !IsValidSkillSlot(proc->leftUnit, newSel));
            if (newSel >= 0)
                proc->leftSelected = newSel;
        } else {
            int newSel = proc->rightSelected;
            do { newSel--; } while (newSel >= 0 && !IsValidSkillSlot(proc->rightUnit, newSel));
            if (newSel >= 0)
                proc->rightSelected = newSel;
        }
    }

    // Process DPAD_DOWN: move to the next valid skill.
    if (keys & DPAD_DOWN) {
        if (proc->activeSide == 0) {
            int newSel = proc->leftSelected;
            do { newSel++; } while (newSel < SKILL_SLOT_COUNT && !IsValidSkillSlot(proc->leftUnit, newSel));
            if (newSel < SKILL_SLOT_COUNT)
                proc->leftSelected = newSel;
        } else {
            int newSel = proc->rightSelected;
            do { newSel++; } while (newSel < SKILL_SLOT_COUNT && !IsValidSkillSlot(proc->rightUnit, newSel));
            if (newSel < SKILL_SLOT_COUNT)
                proc->rightSelected = newSel;
        }
    }

    // Process LEFT/RIGHT input: attempt to switch sides only if the target menu has any valid skills.
    if (keys & (DPAD_LEFT | DPAD_RIGHT)) {
        int otherSide = proc->activeSide ^ 1;
        if (otherSide == 0 && HasAnySkill(proc->leftUnit)) {
            proc->activeSide = 0;
            // If the current selected slot on the left is empty, select the first valid one.
            if (!IsValidSkillSlot(proc->leftUnit, proc->leftSelected)) {
                for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
                    if (IsValidSkillSlot(proc->leftUnit, i)) {
                        proc->leftSelected = i;
                        break;
                    }
                }
            }
        }
        else if (otherSide == 1 && HasAnySkill(proc->rightUnit)) {
            proc->activeSide = 1;
            if (!IsValidSkillSlot(proc->rightUnit, proc->rightSelected)) {
                for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
                    if (IsValidSkillSlot(proc->rightUnit, i)) {
                        proc->rightSelected = i;
                        break;
                    }
                }
            }
        }
    }

    // Process A button: first press selects, second does transfer.
    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        if (proc->state == 0) {
            proc->selectedColumn = proc->activeSide;
            proc->selectedRow = (proc->activeSide == 0) ? proc->leftSelected : proc->rightSelected;
            proc->state = 1;
            // Switch active side for target selection.
            proc->activeSide ^= 1;
            // Advance the target selection to the next empty slot, if available.
            if (proc->activeSide == 0) {
                int newSel = proc->leftSelected;
                bool found = false;
                for (int i = proc->leftSelected + 1; i < SKILL_SLOT_COUNT; i++) {
                    if (!IsValidSkillSlot(proc->leftUnit, i)) {
                        newSel = i;
                        found = true;
                        break;
                    }
                }
                proc->leftSelected = found ? newSel : 0;
            } else {
                int newSel = proc->rightSelected;
                bool found = false;
                for (int i = proc->rightSelected + 1; i < SKILL_SLOT_COUNT; i++) {
                    if (!IsValidSkillSlot(proc->rightUnit, i)) {
                        newSel = i;
                        found = true;
                        break;
                    }
                }
                proc->rightSelected = found ? newSel : 0;
            }
        } else {
            // State 1: perform the action.
            if (proc->selectedColumn == 0) {
                // Source left, target right.
                int leftSkill = GET_SKILL(proc->leftUnit, proc->selectedRow);
                int rightSkill = GET_SKILL(proc->rightUnit, proc->rightSelected);
                if (IsValidSkillSlot(proc->rightUnit, proc->rightSelected)) {
                    // Both slots have skills: perform a direct swap.
                    SET_SKILL(proc->leftUnit, proc->selectedRow, rightSkill);
                    SET_SKILL(proc->rightUnit, proc->rightSelected, leftSkill);
                    {
                        struct Text temp = proc->leftText[proc->selectedRow];
                        proc->leftText[proc->selectedRow] = proc->rightText[proc->rightSelected];
                        proc->rightText[proc->rightSelected] = temp;
                    }
                } else {
                    // Target slot is empty: transfer leftSkill and shift up left menu.
                    SET_SKILL(proc->rightUnit, proc->rightSelected, leftSkill);
                    proc->rightText[proc->rightSelected] = proc->leftText[proc->selectedRow];
                    
                    // Shift up the left menu starting at the selected row:
                    for (int i = proc->selectedRow; i < SKILL_SLOT_COUNT - 1; i++) {
                        int nextSkill = GET_SKILL(proc->leftUnit, i + 1);
                        SET_SKILL(proc->leftUnit, i, nextSkill);
                        proc->leftText[i] = proc->leftText[i + 1];
                    }
                    // Clear the last slot.
                    SET_SKILL(proc->leftUnit, SKILL_SLOT_COUNT - 1, 0);
                    ClearText(&proc->leftText[SKILL_SLOT_COUNT - 1]);
                    // Reset left selection to the first valid slot.
                    {
                        int newSel = 0;
                        for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
                            if (IsValidSkillSlot(proc->leftUnit, i)) {
                                newSel = i;
                                break;
                            }
                        }
                        proc->leftSelected = newSel;
                    }
                }
            } else {
                // Source right, target left.
                int rightSkill = GET_SKILL(proc->rightUnit, proc->selectedRow);
                int leftSkill = GET_SKILL(proc->leftUnit, proc->leftSelected);
                if (IsValidSkillSlot(proc->leftUnit, proc->leftSelected)) {
                    // Both slots have skills: perform a direct swap.
                    SET_SKILL(proc->rightUnit, proc->selectedRow, leftSkill);
                    SET_SKILL(proc->leftUnit, proc->leftSelected, rightSkill);
                    {
                        struct Text temp = proc->rightText[proc->selectedRow];
                        proc->rightText[proc->selectedRow] = proc->leftText[proc->leftSelected];
                        proc->leftText[proc->leftSelected] = temp;
                    }
                } else {
                    // Target slot is empty: transfer rightSkill and shift up right menu.
                    SET_SKILL(proc->leftUnit, proc->leftSelected, rightSkill);
                    proc->leftText[proc->leftSelected] = proc->rightText[proc->selectedRow];
                    
                    // Shift up the right menu starting at the selected row:
                    for (int i = proc->selectedRow; i < SKILL_SLOT_COUNT - 1; i++) {
                        int nextSkill = GET_SKILL(proc->rightUnit, i + 1);
                        SET_SKILL(proc->rightUnit, i, nextSkill);
                        proc->rightText[i] = proc->rightText[i + 1];
                    }
                    SET_SKILL(proc->rightUnit, SKILL_SLOT_COUNT - 1, 0);
                    ClearText(&proc->rightText[SKILL_SLOT_COUNT - 1]);
                    // Reset right selection to the first valid slot.
                    {
                        int newSel = 0;
                        for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
                            if (IsValidSkillSlot(proc->rightUnit, i)) {
                                newSel = i;
                                break;
                            }
                        }
                        proc->rightSelected = newSel;
                    }
                }
            }
            proc->state = 0;
            SkillSwapTradeMenu_Update(proc);
        }
    }

    // Process B button: cancel selection or exit.
    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        if (proc->state == 1) {
            proc->state = 0;
            proc->activeSide ^= 1;
        } else {
            BG_Fill(gBG0TilemapBuffer, 0);
            BG_Fill(gBG1TilemapBuffer, 0);
            BG_Fill(gBG2TilemapBuffer, 0);
            BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
            Proc_End(proc);
            return;
        }
    }

    // Update menus.
    SkillSwapTradeMenu_Update(proc);

    // Fix: when the right slot is active, redraw its skill entry after drawing the UI hand.
    if (proc->state == 0) {
        int activeSide = proc->activeSide;
        int row = (activeSide == 0) ? proc->leftSelected : proc->rightSelected;
        if (activeSide == 0) {
            // Left side: draw hand then redraw so icon/text appear on top.
            DrawHand(activeSide, row, false);
            RedrawSlot(activeSide, row, proc);
        } else {
            // Right side: redraw first then draw hand so underlying art is on top.
            DrawHand(activeSide, row, false);
            RedrawSlot(activeSide, row, proc);
        }
        // Also refresh the opposite menu slot.
        RedrawSlot(activeSide ^ 1, row, proc);
    } 
    else {
        // State 1: draw frozen highlight and refresh affected slots.
        DrawHand(proc->selectedColumn, proc->selectedRow, true);
        RedrawSlot(proc->selectedColumn, proc->selectedRow, proc);
        RedrawSlot(proc->selectedColumn ^ 1, proc->selectedRow, proc);
        // Then update active hand overlay.
        if (proc->activeSide == 0) {
            DrawHand(0, proc->leftSelected, false);
            RedrawSlot(0, proc->leftSelected, proc);
        } else {
            DrawHand(1, proc->rightSelected, false);
            RedrawSlot(1, proc->rightSelected, proc);
        }
    }
}

static const struct ProcCmd ProcScr_SkillSwapTradeMenu[] = {
    PROC_REPEAT(SkillSwapTradeMenu_OnLoop),
    PROC_END
};

void StartSkillSwapTradeMenu(struct Unit * leftUnit, struct Unit * rightUnit)
{
    StartSysBrownBox(6, 0x4800, 0x08, 0x800, 0x400, Proc_Find(gProcScr_PlayerPhase));
    EnableSysBrownBox(0, -40, -1, 1);
    EnableSysBrownBox(1, 184, -1, 0);
    int leftPosition = ((8 * UNIT_PANEL_WIDTH) - GetStringTextLen(GetStringFromIndex(gActiveUnit->pCharacterData->nameTextId))) / 2;
    int rightPosition = ((8 * UNIT_PANEL_WIDTH) - GetStringTextLen(GetStringFromIndex(GetUnit(gActionData.targetIndex)->pCharacterData->nameTextId))) / 2;
    PutDrawText(NULL, gBG1TilemapBuffer + TILEMAP_INDEX(0, 0), 0, leftPosition, UNIT_PANEL_WIDTH, GetStringFromIndex(gActiveUnit->pCharacterData->nameTextId));
    PutDrawText(NULL, gBG1TilemapBuffer + TILEMAP_INDEX(24, 0), 0, rightPosition, UNIT_PANEL_WIDTH, GetStringFromIndex(GetUnit(gActionData.targetIndex)->pCharacterData->nameTextId));

    struct SkillSwapTradeMenuProc *proc = Proc_StartBlocking(ProcScr_SkillSwapTradeMenu, Proc_Find(gProcScr_PlayerPhase));
    proc->leftUnit = leftUnit;
    proc->rightUnit = rightUnit;
    proc->leftSelected = 0;
    proc->rightSelected = 0;
    proc->activeSide = 0; // start with left active
    proc->state = 0;      // no skill selected yet
    proc->selectedColumn = -1;
    proc->selectedRow = -1;

    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        // Allocate enough width for the skill name.
        InitText(&proc->leftText[i], 16);
        InitText(&proc->rightText[i], 16);
    }

    ResetIconGraphics();
    LoadIconPalettes(4); // TODO: palette id constant
}

u8 SkillSwap_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentSameFaction))
		return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

static u8 SkillSwap_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_SkillSwap;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 SkillSwap_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
 if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForAdjacentSameFaction(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, SkillSwap_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
}

static void callback_exec(ProcPtr proc)
{	
    struct Unit * unit_tar = GetUnit(gActionData.targetIndex);
    StartSkillSwapTradeMenu(gActiveUnit, unit_tar);
}

bool Action_SkillSwap(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif