# Menu skills

> Text file: `Contents/Texts/Source/texts/Skills_Menu.txt`  ·  Code: `Kernel/Wizardry/SkillEffects/MenuSkills/`  ·  Hook: `gSkillMenuInfos` (per-SID `.isAvailable` / `.onSelected`) and `gSkillActionFuncTable` (per-SID action, indexed by SID)

## What it is

A menu skill adds a command to the "Skills" submenu of the unit action menu (`MSG_MenuSkill_UM_NAME`, `Data/UnitMenu/Source/UnitMenu.c:190`). Choosing it either runs its action at once (instant, e.g. `Hide`) or first opens a unit/tile selection (targeted, e.g. `Pivot`). The action then runs through the unit-action pipeline as `CONFIG_UNIT_ACTION_EXPA_ExecSkill`, and can change map state (unit positions, statuses, stats, items, tiles).

The AI can also use menu skills, through a fallback that only runs when `menu_skill_ai_use` is set (off by default). Each SID must be added to the AI switch by hand.

## How the engine calls it

### Player path

1. Unit action menu. `gUnitActionMenuItemsRework` (`Data/UnitMenu/Source/UnitMenu.c:138`) has a "Skills" row (`UnitMenu.c:190`) whose Usability and OnSelected are `pr_UpperMenuSkill_Usability` / `pr_UpperMenuSkill_OnSelected`, jumped to `UpperMenuSkill_Usability` (`Kernel/Wizardry/SkillSys/kernel/SkillMenu.c:80`) and `UpperMenuSkill_OnSelected` (`SkillMenu.c:94`) via `Kernel/Preload/KernelReloc.event:82-83`.
2. Submenu. `UpperMenuSkill_OnSelected` opens `sUnitSkillMenuDef` (`SkillMenu.c:70`), a fixed list of 4 rows (`sUnitSkillMenuItems`, `SkillMenu.c:14`; `UNIT_MENU_SKILL_AMOUNT` is 4 at `include/kernel/skill-system.h:307`). Every row's Usability is `MenuSkills_Usability` (`SkillMenu.c:165`). On row 0 it calls `GenerateUnitMenuSkillList` (`SkillMenu.c:122`, called at `:172-173`).
3. Slot fill. `GenerateUnitMenuSkillList` walks `GetUnitSkillList(unit)` in list order and keeps every SID whose `gpSkillMenuInfos[sid]` is non-NULL and has a non-NULL `isAvailable` (`SkillMenu.c:137-148`), up to 4 slots. It does not call Usability at this point.
4. Per-row dispatch. `MenuSkills_Usability` (`SkillMenu.c:181`) calls your `isAvailable(self, number)`. `MenuSkills_OnSelected` (`SkillMenu.c:232-238`) calls your `onSelected(menu, item)`. `MenuSkills_OnDraw` (`SkillMenu.c:221`) draws the label. `MenuSkills_OnHelpBox` (`SkillMenu.c:152-158`) shows `helpMsgId`. The unit-menu help-box hook `UnitActionMenu_OnHelpBoxRe` (`Kernel/Wizardry/UnitMenu/Source/hooks.c:5`, wired at `Kernel/Wizardry/UnitMenu/UnitMenu.event:13`) forwards to `MenuSkills_OnHelpBox`.
5. Your OnSelected. It either returns menu-end flags at once (instant) or starts a target selection (targeted). In both cases it selects the action by writing `gActionData.unk08 = SID_<Name>` and `gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill`.
6. Engine. `ApplyUnitAction` (`Kernel/Wizardry/UnitAction/Source/UnitAction.c:36`) sets `US_HAS_MOVED` on the subject (`UnitAction.c:41`) and calls `gpUnitActionTable[gActionData.unitActionType]` (`UnitAction.c:58`). The table `gUnitActionTable` (`Kernel/Wizardry/UnitAction/Source/UnitActionData.c:13`) maps `[CONFIG_UNIT_ACTION_EXPA_ExecSkill]` to `ActionExecSkill` (`UnitActionData.c:34`). `CONFIG_UNIT_ACTION_EXPA_ExecSkill` is `0x23` (`include/configs/configs.h:16`). The vanilla caller of `ApplyUnitAction` after the menu closes was not traced (unverified).
7. Skill dispatch. `ActionExecSkill` (`Kernel/Wizardry/SkillSys/kernel/SkillAction.c:4-11`) reads the SID from `gActionData.unk08` and calls the function at `gpSkillActionFuncTable[sid]` with the proc. The table is `gSkillActionFuncTable` (`Kernel/Data/SkillSys/Data/SkillActionInfo.c:5`).
8. Your `Action_<Name>` normally calls `NewMuSkillAnimOnActiveUnit(sid, callback_anim, callback_exec)` (`Kernel/Wizardry/SkillSys/SkillMapAnim/MapAnimSkillMini.c:207`). `callback_exec` applies the effect (Pivot: `Kernel/Wizardry/SkillEffects/MenuSkills/Pivot.c:81-97`).

Targeted variant (`Pivot.c`):
- Usability (`Pivot.c:26-35`): returns `MENU_NOTSHOWN` if `US_CANTOING`; returns `MENU_DISABLED` if `HasSelectTarget(gActiveUnit, MakePivotTargetListForAdjacentAlly)` is false.
- OnSelected (`Pivot.c:56-75`): if disabled, `MenuFrozenHelpBox(menu, MSG_SKILL_Pivot_FRtext)` and `MENU_ACT_SND6B` (`:58-62`). Otherwise it builds the target list (`:66`), then `StartSubtitleHelp(NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Pivot_OnSelectTarget), GetStringFromIndex(MSG_SKILL_Common_Target))` (`:69-71`), and returns `MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A` (`:74`). The menu closes and the selection runs.
- Target callback `Pivot_OnSelectTarget` (`Pivot.c:37-54`): stores `gActionData.targetIndex`, `xOther`, `yOther`, sets `unk08` and `unitActionType` (`:49-50`), and returns `TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS` (`:52-53`). After that the engine runs steps 6-8.

Instant variant (`Hide.c`):
- Usability (`Hide.c:14-17`) always returns `MENU_ENABLED`.
- OnSelected (`Hide.c:19-31`) returns `MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR` after setting `unk08` and `unitActionType` (`:27-28`). Then the engine runs steps 6-8.

### AI path

- Gate. `AiTryDoMenuSkills` (`Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c:95`) returns early unless `gpKernelDesignerConfig->menu_skill_ai_use` is true (`MiscAiSkills.c:103`). The default is `false` (`Data/DesignerConfig/designer-config.c:99`; field at `include/kernel/kernel-lib.h:86`).
- Selection. It scans the acting unit's skill list (`MiscAiSkills.c:106-122`), requires `isAvailable(def, 0) == MENU_ENABLED` (`:121`), then runs `switch (sid)` (`:124`). A SID with no case is skipped (`default: continue;`, `:321`). Self-target cases use `AI_MENU_SELF_CASE` (`AiMenuSkillSetCurrentTile`, `:35`). Targeted cases use `AI_MENU_TARGET_CASE(sid, MakeList)` (`AiMenuSkillSetTargetFromList`, `:49`).
- Call sites. `AiTryDoMenuSkills` is called from `DecideScriptA` (`Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c:708`), `DecideScriptB` (`:747`), `DecideHealOrEscape` (`:807`) and `AiTryDoStaff` (`:839`), after the higher-priority AI passes.
- Execution. `AiAction_MenuSkill` (`MiscAiSkills.c:329-341`) sets `unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill` and the target and move fields from the AI decision, then calls `ApplyUnitAction` (`:340`). Steps 6-8 follow.
- Pivot has no case in the switch, so the AI never selects it. Whether a Pivot-style "move the target" action behaves correctly when driven by an AI decision is unverified.

## Where to look

Files beyond the common set (see [Common.md](Common.md)):

| File | Role |
|---|---|
| `Kernel/Wizardry/SkillEffects/MenuSkills/<Name>.c` | Usability, OnSelected, target callback, `callback_exec`, `Action_<Name>`. Guard: `#if defined(SID_<Name>) && (COMMON_SKILL_VALID(SID_<Name>))` |
| `Kernel/Wizardry/SkillEffects/MenuSkills/<Name>.lyn.event` | lyn output of the compiled `.c` (rule `%.lyn.event: %.o`, `Tools/FE-CLib-Mokha/wizardry.mk:31-32`). Committed next to the `.c`. Its `ORG CURRENTOFFSET+$xx` lines are generated, do not hand-edit |
| `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event` | `#include "MenuSkills/<Name>.lyn.event"` (the MenuSkills block runs from line 35; Pivot is at line 67) |
| `Kernel/Data/SkillSys/Data/SkillMenuInfo.c` | `gSkillMenuInfos` entry: `[SID_<Name>] = &(const struct MenuItemDef) {...}`. Pivot at lines 577-590 |
| `Kernel/Data/SkillSys/Data/SkillActionInfo.c` | `gSkillActionFuncTable` entry: `[SID_<Name>] = Action_<Name>,`. Pivot at lines 138-140 |
| `include/kernel/skill-system.h` | Prototypes: `<Name>_Usability` and `<Name>_OnSelected` (Pivot at `:557-558`, Hide at `:603-604`), `Action_<Name>` (`:710`, `:733`) |
| `Contents/Texts/Source/texts/Skills_Menu.txt` | Message keys (table below) |
| `Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c` | AI switch (`:124`). Edit only if the AI should use the skill |
| `include/kernel/playst-expa.h` | `PLAYSTEXPA_BIT_<Name>_Used` enum entry, only for once-per-chapter limits. The enum is at `:5-41` with 34 bits used; `PLAYSTEXPA_BIT_MAX` is 128 |
| `Kernel/Wizardry/PlayStExpa/Source/PlayStExpa.c` | `ChapterInit_ResetPlayStExpa` (`:10-13`) clears all bits. Read only; hooked in `Kernel/Wizardry/ChapterInitHook/data.event:10` |

Read-only engine files for this type: `Kernel/Wizardry/SkillSys/kernel/SkillMenu.c`, `Kernel/Wizardry/SkillSys/kernel/SkillAction.c`, `Kernel/Wizardry/UnitAction/Source/UnitAction.c`.

Message keys used by menu skills (`Skills_Menu.txt` has 100 NAME keys, 100 DESC keys and 21 FRtext keys, counted by grep):

| Key | Used for | Evidence |
|---|---|---|
| `MSG_SKILL_<Name>_NAME` | `.nameMsgId`. When non-zero the row label is the skill name from `GetSkillNameStr(sid)`, and `.name` is ignored | `SkillMenuInfo.c:580`, `SkillMenu.c:207-212` |
| `MSG_SKILL_<Name>_DESC` | `.helpMsgId`, shown in the help box | `SkillMenuInfo.c:581`, `SkillMenu.c:157` |
| `MSG_SKILL_<Name>_FRtext` | Help box when OnSelected sees `MENU_DISABLED` (per skill) | `Pivot.c:60`, `Skills_Menu.txt:54` |
| `MSG_SKILL_CommonFail` (shared) | Generic "cannot be used for now" | `Skills_Menu.txt:1`, `Hide.c:23`, `FocusEnergy.c:62` |
| `MSG_SKILL_CannotUseAgain_FRtext` (shared) | Once-per-chapter failure | `GraceOfFire.c:25`, `GraceOfWater.c:25`, `DivineReprieve/Source/DivineReprieve.c:26` |
| `MSG_SKILL_Common_Target` (shared) | Target-selection prompt | `Pivot.c:71`, `Refuge.c:127` |
| `MSG_SKILL_<Name>_Target` (per skill) | Custom target prompt | `Portal.c:253`, `Skills_Menu.txt:103` |
| `MSG_<Family>_NO_USE` (shared by a family) | Once-per-map failure for a skill family | `MSG_Persuade_NO_USE` in `Persuade.c:54` and `PersuadePlus.c:53`; `MSG_Ballistary_NO_USE` in `Ballistary.c:30` |
| `MSG_MenuSkill_UM_NAME` / `MSG_MenuSkill_UM_DESC` | The "Skills" row in the unit menu, not per skill | `Data/UnitMenu/Source/UnitMenu.c:191-192`, `Skills_Menu.txt:13-18` |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the variant. Instant: copy the Instant template. Targeted: Usability gates on `HasSelectTarget` and OnSelected calls `NewTargetSelection_Specialized`. Copy the Targeted template.
3. Create `Kernel/Wizardry/SkillEffects/MenuSkills/<Name>.c` from the template, guarded by `#if defined(SID_<Name>) && (COMMON_SKILL_VALID(SID_<Name>))`. Usability returns `MENU_NOTSHOWN` (hidden), `MENU_DISABLED` (shown greyed, OnSelected must handle it) or `MENU_ENABLED`.
4. Generate `<Name>.lyn.event` from the compiled `.o` (`Tools/FE-CLib-Mokha/wizardry.mk:31-32`) and commit it next to the `.c`. Unverified: whether the main build regenerates it automatically. The committed `Pivot.lyn.event` suggests it is checked in.
5. Add `#include "MenuSkills/<Name>.lyn.event"` to the MenuSkills block of `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event` (block lines 35-123).
6. Register the row in `Kernel/Data/SkillSys/Data/SkillMenuInfo.c` and the action in `Kernel/Data/SkillSys/Data/SkillActionInfo.c`. Each entry sits inside its own `#if (defined(SID_<Name>) && COMMON_SKILL_VALID(SID_<Name>))` block.
7. Add prototypes to `include/kernel/skill-system.h`.
8. Add texts to `Contents/Texts/Source/texts/Skills_Menu.txt`: `MSG_SKILL_<Name>_NAME`, `MSG_SKILL_<Name>_DESC`, and `MSG_SKILL_<Name>_FRtext` if OnSelected uses it.
9. Optional, once per chapter: add `PLAYSTEXPA_BIT_<Name>_Used` to the enum in `include/kernel/playst-expa.h` (before `PLAYSTEXPA_BIT_USED`, `:41`). Check it in Usability (`MENU_DISABLED`) and set it in the exec path. GraceOfFire sets it in `callback_exec` (`GraceOfFire.c:51`).
10. Optional, AI: add `AI_MENU_SELF_CASE(SID_<Name>);` or `AI_MENU_TARGET_CASE(SID_<Name>, MakeTargetList...);` to the switch in `Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c`, inside the `#if` pattern used there. The AI only runs when `menu_skill_ai_use` is true.

## Template

### Targeted (adapted from `Kernel/Wizardry/SkillEffects/MenuSkills/Pivot.c`)

Source: `Kernel/Wizardry/SkillEffects/MenuSkills/<Name>.c`

```c
#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "action-expa.h"

#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))

extern void ForEachAdjacentUnit(int x, int y, void (*)(struct Unit *));

/* Target list: adjacent allies of gSubjectUnit (same shape as MakePivotTargetListForAdjacentAlly) */
static void Example_TryAddTarget(struct Unit * unit)
{
    if (unit == gSubjectUnit)
        return;

    if (!AreUnitsAllied(gSubjectUnit->index, unit->index))
        return;

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

void Example_MakeTargetList(struct Unit * unit)
{
    InitTargets(0, 0);
    gSubjectUnit = unit;
    ForEachAdjacentUnit(unit->xPos, unit->yPos, Example_TryAddTarget);
}

u8 Example_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, Example_MakeTargetList))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

static u8 Example_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;
    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_Example;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
        TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Example_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_Example_FRtext);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    Example_MakeTargetList(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Example_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void Example_callback_anim(ProcPtr proc)
{
}

static void Example_callback_exec(ProcPtr proc)
{
    struct Unit * targetUnit = GetUnit(gActionData.targetIndex);

    /* Effect goes here. gActiveUnit = actor, targetUnit = chosen unit, gActionData.xOther/yOther = chosen tile. */
    (void)targetUnit;
}

bool Action_Example(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, Example_callback_anim, Example_callback_exec);
    return true;
}

#endif
```

Registration (both variants):

```c
/* Kernel/Data/SkillSys/Data/SkillMenuInfo.c */
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
    [SID_Example] = &(const struct MenuItemDef) {
        .name = "　Example",
        .nameMsgId = MSG_SKILL_Example_NAME,
        .helpMsgId = MSG_SKILL_Example_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Example_Usability,
        .onDraw = NULL,
        .onSelected = Example_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

/* Kernel/Data/SkillSys/Data/SkillActionInfo.c */
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
    [SID_Example] = Action_Example,
#endif

/* include/kernel/skill-system.h */
u8 Example_Usability(const struct MenuItemDef * def, int number);
u8 Example_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
bool Action_Example(ProcPtr parent);
```

```event
; Kernel/Wizardry/SkillEffects/MiscSkillEffects.event, inside the MenuSkills block
#include "MenuSkills/Example.lyn.event"
```

Adapted from `Pivot.c` (`:15-75`, `:81-97`, `:225-229`) and `SkillMenuInfo.c:577-590`.

### Instant (adapted from `Kernel/Wizardry/SkillEffects/MenuSkills/Hide.c`)

Source: `Kernel/Wizardry/SkillEffects/MenuSkills/<Name>.c`

```c
#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "action-expa.h"

#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))

u8 Example_Usability(const struct MenuItemDef * def, int number)
{
    return MENU_ENABLED;
}

u8 Example_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Example;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void Example_callback_anim(ProcPtr proc)
{
}

static void Example_callback_exec(ProcPtr proc)
{
    /* Effect goes here. Hide uses SetUnitStatus(gActiveUnit, NEW_UNIT_STATUS_HIDE). */
}

bool Action_Example(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, Example_callback_anim, Example_callback_exec);
    return true;
}

#endif
```

Adapted from `Hide.c` (`:13-52`). Registration and event include are the same as the Targeted template. Hide's own callback_anim plays `ProcScr_DanceringAnim` (`Hide.c:33-39`) if a dance animation is wanted.

## Worked examples

- `Hide`, `Kernel/Wizardry/SkillEffects/MenuSkills/Hide.c`: instant, self-only, no use limit (Usability always `MENU_ENABLED`). Exec sets a unit status.
- `Pivot`, `Kernel/Wizardry/SkillEffects/MenuSkills/Pivot.c`: targeted (adjacent ally), gated on `HasSelectTarget`. Exec moves the actor to the far side of the target (`callback_exec`, `:81-97`). Sets `US_HAS_MOVED | US_CANTOING` on exec (`:83`).
- `GraceOfFire`, `Kernel/Wizardry/SkillEffects/MenuSkills/GraceOfFire.c`: instant, once per chapter. Uses `PLAYSTEXPA_BIT_GraceOfFire_Used` (`:16-17`, set at `:51`) and `MSG_SKILL_CannotUseAgain_FRtext` (`:25`).
- `Ballistary`, `Kernel/Wizardry/SkillEffects/MenuSkills/Ballistary.c`: once per map (text), with the bit set in OnSelected (`:43`). The effect is done in OnSelected (`:37-38`), so `Action_Ballistary` is empty (`:58-63`).

## Gotchas

- Four-row cap counts hidden skills. `GenerateUnitMenuSkillList` (`Kernel/Wizardry/SkillSys/kernel/SkillMenu.c:137-148`) fills the 4 slots with any skill that has a non-NULL `isAvailable`, even if its Usability is currently `MENU_NOTSHOWN` (for example after the unit has moved). A unit with 4 such skills can lose a usable one from the submenu.
- Off-by-one. The loop breaks on `cnt > UNIT_MENU_SKILL_AMOUNT` (`SkillMenu.c:141`), not `>=`. A fifth qualifying skill is written to `UnitMenuSkills[4]`, one past the end of `UnitMenuSkills[UNIT_MENU_SKILL_AMOUNT]` (`include/kernel/skill-system.h:307-308`). Not fixed here. The impact on the memory after the array is unverified.
- Action id. Use `CONFIG_UNIT_ACTION_EXPA_ExecSkill` (`0x23`, `include/configs/configs.h:16`). The `#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill` / `#define ... 20` block in the menu-skill files (`Pivot.c:11-13`) is dead code, because `common-chax.h` (`include/common-chax.h:7`) includes `configs.h` first. Do not copy it.
- The live registration of the action is the table entry at `Kernel/Wizardry/UnitAction/Source/UnitActionData.c:34`. The `InjectUnitAction(CONFIG_UNIT_ACTION_EXPA_ExecSkill, ActionExecSkill)` line in `Kernel/Wizardry/SkillSys/SkillSys.event:23` is commented out.
- Use-limit gating. `US_CANTOING` is an alias of `US_HAS_MOVED` (`Tools/FE-CLib-Mokha/include/bmunit.h:237`). Checking it in Usability hides the skill once the unit has moved or acted this turn. `ApplyUnitAction` sets `US_HAS_MOVED` on every action (`UnitAction.c:41`). `Hide.c` does not check it (`Hide.c:14-17`), so Hide is selectable regardless of state. Whether that is intended is unverified.
- Use-limit gating, per chapter. No per-turn counter exists among `MenuSkills/*.c` (a grep for `turn` finds only a comment in `Hellscape.c`). The per-chapter limit is a `PLAYSTEXPA_BIT_*` bit, cleared by `ChapterInit_ResetPlayStExpa` (`Kernel/Wizardry/PlayStExpa/Source/PlayStExpa.c:10-13`). Texts such as `MSG_Persuade_NO_USE` say "this map". Whether the chapter-init hook also runs per map is unverified.
- A `MENU_DISABLED` row still calls OnSelected. Check `item->availability == MENU_DISABLED` first (`Pivot.c:58`, `Hide.c:21`), or the action runs.
- Targeted: write the action id in the target callback (`Pivot.c:49-50`), not in OnSelected. The target callback returns the `TARGETSELECTION_ACTION_*` flags that Pivot uses (`Pivot.c:52-53`). Which flag does what was not verified.
- Label. With `.nameMsgId` set, the row text is `GetSkillNameStr(sid)` (`SkillMenu.c:207-210`). The `.name` string is only drawn when `nameMsgId` is 0 (`SkillMenu.c:211-212`).
- `UnitActionMenu_CancelRe` (`Kernel/Wizardry/UnitMenu/Source/hooks.c:10-14`, which calls `ResetSkillLists`) is not wired: its registration is commented out in `Kernel/Wizardry/UnitMenu/UnitMenu.event:10`.
- AI. A menu skill is never chosen by the AI unless it has a case in the switch (`MiscAiSkills.c:124`). The AI is off by default (`designer-config.c:99`). Pivot is not in the switch.
- Grep scope. `.claude/worktrees/` holds stale copies of many of these files with different content. Use `grep -rn --exclude-dir=.claude` to avoid them.
