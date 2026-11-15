# Double Badge

A "×2" or "×4" badge in battle animations that shows how many times a side strikes in one exchange.

## What it shows

A badge sits just above a side's Hit/Dmg/Crit box when that side strikes more than once per exchange.

| Badge | Meaning |
|---|---|
| ×2 | The side doubles (a follow-up attack) or has a brave weapon. |
| ×4 | The side does both. |

No badge shows when a side strikes once, or when it cannot strike (no weapon, or the target cannot counter).

It shows for either side. In the arena you can see whether the opponent doubles you before you commit.

## Counting

`BattleUnwind` (`Kernel/Wizardry/BattleSys/Source/BattleOrder.c`) calls `RecordDoubleBadge` with the battle's order, `BattleUnwindConfig[gBattleFlagExt.round_mask]`. For each side, it counts the steps in that order and multiplies by `GetBattleUnitHitCount` (2 with a brave weapon).

- It runs once per real battle. Simulated battles return earlier.
- It runs before any hit, so HP-dependent effects are as they are at the start of the battle.
- A duel (`HasDuel`) records 0, so no badge shows.
- It does not call `CheckCanContinueAttack`. That call charges gaiden magic's HP cost, so the count does not use it.

The values live in `gDoubleBadge` (`include/kernel/double-badge.h`). It is 4 bytes of RAM, allocated with `_kernel_malloc2`. `strikes[0]` is for `gBattleActor` and `strikes[1]` is for `gBattleTarget`.

## Drawing

The drawing code is in `Kernel/Wizardry/DoubleBadge/Source/DoubleBadge.c`.

`NewEkrGauge` (`Kernel/Wizardry/HPBarPalettes/HPBarPalettes.c`) starts `ProcScr_DoubleBadge` as a child of the battle HUD's proc (`gpProcEkrGauge`). So the badge ends with the HUD.

The badge is the battle forecast's own "xN" sprite. It is `Gfx_BKSEL[n]`, a 16x16 sprite made of four tiles (TL, TR, BL, BR), with n = 2 to 9. Tile 10 is "xN" for more than 9. The first 16 colours of `Pal_BKSEL` are used. Both are in `Kernel/Wizardry/BattleSys/data.event`. The badge is drawn on OBJ palette 15.

Each frame, the badge draws unless the HUD is hidden (its byte `+0x2A` is 1).

- Top at y 92. The boxes' Hit rows are at y 112.
- Left side at x 24. Right side at x 222, right-aligned over the digit columns.
- The position is offset by the HUD's slide and shake (proc `+0x32` and `+0x3A`).
- It bobs: 0, -1, -2, -1 pixels, one step every 8 frames (`GetGameClock() >> 3`).

### Tiles

The battle screen uses 2D OBJ mapping. The left badge uses tiles 988, 989, 1020 and 1021. The right badge uses tiles 990, 991, 1022 and 1023. They are rewritten every frame the badge shows (`RegisterDataMove`), because spell effects may use that VRAM.

### Palette and hiding

OBJ palette 15 is shared with the kernel's skill-name and anim-number popups (`OBPAL_CHAX_ANIMNUM`). So the badge hides while any of these is true: `EfxSkillExists()`, `EfxSkillBoxExists()`, `EkrSkillExists()`, `EfxAnimNumberExists()` or `CritCutinExists()`. For example, it hides for about a second while a damage number pops up. When it shows again, it reloads its palette, but only if palette 15 differs.

## Limits

- The count follows the battle's order and brave weapons. Other effects that add strikes, such as combo attacks and skills that add hits mid-battle, are not counted.
- Counts above 9 show the "xN" sprite (tile 10), not the number.

## Testing

Checked by hand in the emulator on the prologue. Eirika doubles a soldier, so ×2 shows over her box and none shows over his. The bob and the hide during the damage popup were also checked in the emulator.

`Tools/cloud/smoketest.py` has no test for this yet.

`Tools/cloud/smoketest.py` has no test for this yet.
