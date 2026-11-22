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

The badge is the battle forecast's own "xN" sprite. It is `Gfx_BKSEL[n]`, a 16x16 sprite made of four tiles (TL, TR, BL, BR), with n = 2 to 9. Tile 10 is "xN" for more than 9. The first 16 colours of `Pal_BKSEL` are used. Both are in `Kernel/Wizardry/BattleSys/data.event`. The badge is drawn on OBJ palette 6.

Each frame, the badge draws unless the HUD is hidden (its byte `+0x2A` is 1) or the animation is a promotion (`gEkrDistanceType == EKR_DISTANCE_PROMOTION`: the last battle's counts would show on the promotion screen).

- Top at y 92. The boxes' Hit rows are at y 112.
- Left side at x 24. Right side at x 222, right-aligned over the digit columns.
- The position is offset by the HUD's slide and shake (proc `+0x32` and `+0x3A`).
- It bobs: 0, -1, -2, -1 pixels, one step every 8 frames (`GetGameClock() >> 3`).

### Tiles

The battle screen uses 2D OBJ mapping. The left badge uses tiles 320, 321, 352 and 353; the right badge 322, 323, 354
and 355. Nothing else in a battle uses them: spells load their sprites at tiles 64-319 (vanilla's largest is 0x2000
bytes from 0x06010800), the HP bars use 384-447 and 461-495, and the two sides' animation sheets 512-1023. They are
rewritten every frame the badge shows (`RegisterDataMove`).

The badge was at 988-991 and 1020-1023 before: the last tiles of the right side's sheet. Vanilla's animations leave
them empty, custom ones need not (Gerik's and Tana's use them): the badge overwrote part of the sprite (Gerik's shoe
went missing as his sword landed) and the sheet overwrote the badge (a garbled "x2").

### Palette and hiding

The badge has OBJ palette 6 to itself: no sprite uses row 6 in a battle (checked every frame of a physical, a critical and a magic battle, a level-up, Myrrh's transformation, and the map before and after). Each frame it shows, it reloads its palette if row 6 differs (the screen's fades rewrite it).

It was on palette 10 until Myrrh's transformation showed that row is taken. Each side's animation has two palette rows, 7-8 on the left and 9-10 on the right, and the second is for special animations: the transformation (banim 0xC4) loads her human palette in the first row and her dragon's in the second, and draws the cocoon and the dragon with the second. The badge reset row 10 to its own colours every frame, so the dragon came out pink until the transformation ended and the dragon animation loaded its own palette.

It was on palette 15 before, which the skill-name and anim-number popups (`OBPAL_CHAX_ANIMNUM`) and the level-up box's portrait also use. So it had to hide while any of them showed: it blinked out for about a second at every hit, and resetting palette 15 drew the level-up portrait in its colours.

It still hides during the critical cut-in (`CritCutinExists()`) and the level-up box (`ProcScr_EkrLevelup`), which cover the HUD.

A side shows it only while that side's animation is on screen (`gBanimValid`) and fights with a weapon (`IA_WEAPON`). A staff, a dance or a refresh has no strikes, and its battle doesn't go through `BattleUnwind`, which records them, so the last fight's counts were left: a heal showed an "x2" on its empty side.

## Limits

- The count follows the battle's order and brave weapons. Other effects that add strikes, such as combo attacks and skills that add hits mid-battle, are not counted.
- Counts above 9 show the "xN" sprite (tile 10), not the number.

## Testing

Checked by hand in the emulator on the prologue. Eirika doubles a soldier, so ×2 shows over her box and none shows over his. The bob was also checked in the emulator, and that the badge stays up while a damage number shows (Tana's battle, `smoketest.py`'s `banim` setup).

`python3 Tools/cloud/smoketest.py --only banim` checks that a heal draws no badge: L'Arachel as a Troubadour heals Seth with stale strike counts written into `gDoubleBadge`, and none of the 128 OAM entries uses the badge's tiles (320, 322). The rest is still checked by hand.
