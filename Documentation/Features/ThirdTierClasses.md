# Third-Tier Classes

Four promoted classes have a third tier, as in FE10:

| Third tier | Class id | Promoted from | Standing sprite (SMS id, size) | Class skills | Battle animation |
|---|---|---|---|---|---|
| Trickster | `CLASS_TRICKSTER` 0x67 | Rogue | 0x7B, 16x16 | Lethality, Lucky Seven | 0xE3 (knives and swords) |
| Marshall | `CLASS_MARSHALL` 0x68 | General, General F | 0x7C, 32x32 | Pavise, Lancefaire (`SID_FaireLance`) | 0xE2 (lances) |
| Trueblade | `CLASS_TRUEBLADE` 0x69 | Swordmaster | 0x7D, 32x32 | Astra, Swordfaire (`SID_FaireSword`) | 0xDF (swords) |
| Archsage | `CLASS_ARCHSAGE` 0x6A | Sage | 0x7E, 16x16 | Flare, Animafaire (`SID_FaireAnima`) | 0xE1, Nuramon's (anima, light, dark) |

Swordmaster F and Sage F have no third tier: the Trueblade and Archsage map sprites are male. The Marshall sprite is
unisex, so both Generals promote to it. The third-tier classes themselves do not promote.

Everything but the class data and the sprites is behind `CONFIG_THIRD_TIER_CLASSES` (`include/configs/configs.h`,
on): the promotions, the battle animations and the cap table. Off, the classes still exist but nothing promotes to
them, and they use their promoted class's vanilla animations.

---

## Class ids

Class ids are a `u8`, but a save keeps a unit's class in 7 bits, so the class table holds 0x01-0x7F and is full. The
four classes reuse vanilla's ballista classes, which nothing uses:

- vanilla's own class data for 0x67-0x6C has no name or description text and no movement costs;
- no unit group in vanilla's ROM or in this build's names them (every unit group that a ROM pointer reaches, 703 in
  vanilla and 837 in the build, scanned for its class bytes), and no world-map sprite event (`PUTSPRITE`) does;
- ballista traps are drawn by their SMS ids (0x58-0x5D, `RefreshUnitSprites` in
  `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c`), not by class;
- nothing in `Kernel`, `Data` or `CustomCampaign` names them.

`CLASS_BLST_REGULAR_USED` to `CLASS_BLST_REGULAR_EMPTY` (0x67-0x6A) are renamed in
`Tools/FE-CLib-Mokha/include/constants/classes.h`, with the vanilla names in comments, as the Halberdier's 0x77 was.
0x6B and 0x6C (`CLASS_BLST_LONG_EMPTY`, `CLASS_BLST_KILLER_EMPTY`) are still free. The other vanilla leftovers are
not: Fallen Peer (0x7B) is Lyon's class in two vanilla scenes, Queen 2 (0x76) has a name, and Cyclops 2 and Elder Bael 2
(0x7C, 0x7D) are in this repo's monster lists (`Data/ClassTypes/ClassTypes.c`).

---

## Promotion

`Data/ClassData/Promotions.event` writes each third tier as the promoted class's only choice in vanilla's
`gPromoJidLut` (`PromoChoices(jid, third tier, 0)`). A promoted unit whose class has a third tier
(`CanUnitPromoteToThirdTier`, `Kernel/Wizardry/Lvupfx/Lvupfx/MapLvup.c`) promotes:

- with a **Master Seal**, at level 10 or more (`CanUnitUsePromotionItem`,
  `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-extra.c`). Vanilla's promotion screen goes straight to the promotion when
  there is one choice. The other promotion items keep their class lists.
- **automatically on reaching level 20**, with `promotion_on_max_level` on (`AutoPromotionDue`, the same file), as an
  unpromoted unit does: after the battle (with or without battle animations), from the post-action entry
  `PostAction_AutoPromotion`.

`CanUnitPromoteToThirdTier` is true for a promoted class (`CA_PROMOTED`) whose `gPromoJidLut` choice is the class id
in its row of `gThirdTierClassPlans` (`Data/ClassData/ThirdTierClasses.c`). It is not "any promoted class with a
`gPromoJidLut` entry": in this repo the Brigand and the Pirate are `CA_PROMOTED` with entries (Warrior, Berserker), and
so are a few monsters (Bonewalker Bow, Tarvos, Gargoyle, the Gorgon eggs); they keep their old rules.

`ApplyUnitPromotion` (`Kernel/Wizardry/Lvup/Source/Promotion.c`) does the rest, as for any promotion: level 1, EXP 0,
the hidden level adds the level reached, the promotion bonuses, ranks carried over by weapon type, and the class's
skills.

### The promotion crash (fixed)

Promoting to a third tier reset the game (the title screen came back) once the unit was back on the map, on every
path: Gilliam made a Marshall with a Master Seal, any General, General F, Rogue, Swordmaster or Sage with a Master
Seal, and the automatic promotion with battle animations on. It needed an animated battle earlier (any battle, the
enemy phase's too); the automatic promotion with animations on always had one just before.

- `NewEkrPopup` (`Kernel/Wizardry/PopupRework/Source/EfxPopup.c`) did not clear `gEkrPopupEnded`, as vanilla's does.
  The animated battle's popups left it set, and the promotion's battle animation (`ekrBattle_WaitForPopup`) ended its
  own popups at once: the class's skills (`gpConstSkillTable_Job`, two for each third tier) were never shown and stayed
  on the popup stack.
- `ClassChgPostConfirmWaitBanimEnd` then started one learned-skill popup for them without its setup: no skill and no
  popup unit (`gpPopupUnit` NULL). `PoprGetLen_UnitName`'s `Assert(IS_EWRAM_PTR(gpPopupUnit))` failed, and the abort
  jumps to address 0 (seen with `gbarun`: `NoCashGBAPrintf("[FATAL] ... Assertion failed: IS_EWRAM_PTR(gpPopupUnit)")`
  from `PoprGetLen_UnitName`, then the BIOS reset vector).
- Two-choice promotions through the class menu (Armor Knight to General) did not crash in the same tests, and had no
  skill left pending afterwards, though `gEkrPopupEnded` stayed set; why that path differs was not worked out.

The fix: `NewEkrPopup` clears `gEkrPopupEnded`; `ClassChgPostConfirmWaitBanimEnd`
(`Kernel/Wizardry/PopupRework/Source/ClassChgPopup.c`, moved out of `MiscFunctions.c`) shows any skill still pending
through `PopR_SetupLearnSkill`, one popup per skill; `TryAddSkillPromotion` (`Kernel/Wizardry/SkillSys/kernel/LoadSkill.c`)
sets the popup unit to the promoted unit.

The automatic promotion without battle animations used to start from the map level-up box (`ManimLevelUp_ScrollOut`).
The battle's result is written to the unit after the box (`BattleApplyUnitUpdates`), which put back the level 20, the
EXP and the stats the promotion had reset: a level-20 Trueblade (or Great Lord, or General) with 99 EXP. Every
automatic promotion now starts from `PostAction_AutoPromotion`, after that write.

---

## Stats

Each class is its promoted class's entry in `Data/ClassData/ClassData.c` with:

- bases: about +2 HP, +1 or +2 a stat, +1 Con, +1 Mov for the Trickster and the Trueblade (6 to 7);
- growths: +5 on the class's three main stats;
- caps: FE10's third tier, from the promoted class's row in `gThirdTierClassPlans`
  ([FE10StatCaps.md](FE10StatCaps.md)): `max*`, `JobMagicInfo` `.cap` (`Data/StrMag/Source/JobMagicInfo.c`) and
  `gClassLuckCap` (`Data/MaxStatus/ClassLuckCap.c`); Con caps are the promoted class's;
- promotion bonuses: +3 HP, +2 on the main stats, +1 on most others (the Archsage: +2 Magic, `JobMagicInfo` `.bonus`);
- the promoted class's weapon ranks, movement, terrain tables, attributes plus `CA_PROMOTED`, portrait, sort order and
  relative power. The Trickster has the Rogue's weapon slots (sword, knife; `Data/WeaponSlots/WeaponSlots.c`), steals
  and opens locks.
- hidden level 30 for a unit loaded in the class (`Data/HiddenLevel/HiddenLevel.c`).

| Class | Bases HP/Str/Mag/Skl/Spd/Def/Res/Con/Mov | Growths HP/Str/Mag/Skl/Spd/Def/Res/Lck | Caps HP/Str/Mag/Skl/Spd/Lck/Def/Res | Promotion HP/Str/Mag/Skl/Spd/Def/Res |
|---|---|---|---|---|
| Trickster | 18/4/2/3/11/3/1/8/7 | 50/10/25/50/40/5/20/45 | 50/28/20/38/40/35/26/25 | 3/1/0/2/2/1/1 |
| Marshall | 23/10/1/5/4/15/4/16/5 | 80/35/0/20/10/28/25/20 | 60/36/20/34/31/30/37/30 | 3/2/0/1/1/2/1 |
| Trueblade | 23/7/2/13/12/6/3/10/7 | 65/30/20/35/35/15/22/25 | 55/32/20/40/40/30/26/24 | 3/2/0/2/2/1/1 |
| Archsage | 22/6/7/6/5/6/7/8/6 | 45/45/50/35/25/10/45/15 | 50/24/38/34/32/30/25/36 | 3/0/2/2/1/1/2 |

---

## Map sprites

The sprites are in `Data/UnitIcon/ThirdTier/` (authors in [Credits](../Credits.md#map-sprites)), installed by
`Data/UnitIcon/ThirdTier.event`. They are indexed PNGs in vanilla's map sprite palette order (`gPal_MapSprite`,
0x859EE20; index 0 transparent), so each team's palette colours them. The source sprites were already in that order,
except the Trueblade's moving sheet (its own order, with near colours: remapped to the nearest index; two of its
colours were the same blue) and the Archsage's standing sheet (RGB: converted, all 15 colours exact).

- **Standing**: in `gUnitIconWaitTable` (`Data/UnitIcon/UnitIconWait.c`) at `SMS_ID_TRICKSTER` to `SMS_ID_ARCHSAGE`
  (0x7B-0x7E, `include/kernel/unit-icon.h`): 3 frames of 16x16 (16x48 sheets) or 32x32 (32x96 sheets,
  `UNIT_ICON_SIZE_32x32`). 0x7F is the last free id.
- **Moving**: vanilla's `unit_icon_move_table` (0x9A2E00, 8 bytes an entry: sheet, AP) is indexed by class id - 1, not
  by SMS id, and already has an entry for every class 0x01-0x7F, so it is not repointed: the four classes' entries
  (the ballistae's) are overwritten. The sheets are 15 frames of 32x32 (left x4, down x4, up x4, selected x3), and each
  uses vanilla Rogue's AP (0x81DF548, one 32x32 object a frame), as the Rogue's own entry does.

Vanilla `GetClassSMSId` (0x8018D90), which `PutUnitSpriteForClassId` draws with (the stat screen's Promotions page,
the Bestiary, the arena roster), reads vanilla's class table inline. Its literal pool now points at `gClassData_NEW`
(`Data/ClassData/Promotions.event`): the two tables' SMS ids differ only for the classes in vanilla's unused slots, which
were drawn as ballistae (and the Halberdier as a blank).

---

## Battle animations

Class-wide, like the Phantom's: each class's `pBattleAnimDef` is a list in `Contents/BattleAnims/BattleAnims.event`
(`gBanimDef_T3*`), its promoted class's vanilla list with the third-tier animation in place of the main weapon's
entries. See [CustomBattleAnimations.md](CustomBattleAnimations.md) for the lists and why the Archsage uses Nuramon's.

---

## Texts

Names and descriptions are at the end of `Contents/Texts/Source/texts/Districts.txt` (`MSG_CLASS_TRICKSTER_NAME` to
`MSG_CLASS_ARCHSAGE_DESC`).

---

## Adding another third-tier class

1. A free class id: 0x6B or 0x6C (the other two ballista classes; check them as above). Rename its constant in
   `Tools/FE-CLib-Mokha/include/constants/classes.h`, keeping the vanilla name in a comment.
2. Its entry in `Data/ClassData/ClassData.c` (replace the ballista's), from the promoted class's, with its row's caps in
   `gThirdTierClassPlans`; its `JobMagicInfo`, `gClassLuckCap` and `gClassPreLoadHiddenLevel` rows; weapon slots if the
   promoted class has a `WeaponSlots.c` entry.
3. Texts at the end of `Districts.txt`.
4. Map sprites: a standing sheet at SMS id 0x7F (the last free one; past it, the table must grow, and vanilla masks
   the id with 0x7F) and an `MMS_ENTRY` line in `Data/UnitIcon/ThirdTier.event`.
5. A battle animation list in `BattleAnims.event` (or the promoted class's list).
6. Its class skills in `Data/SkillSys/Source/SkillTable-job.c`, each row in a SID guard.
7. The promotion: `PromoChoices(from, id, 0)` in `Promotions.event` and the class id in its row of
   `gThirdTierClassPlans` (both, or `CanUnitPromoteToThirdTier` refuses it).
8. `Tools/cloud/build.sh`, then `python3 Tools/cloud/smoketest.py --only t3promo t3promoall` (add the class to
   `test_t3promoall`'s cases).

---

## Units loaded from chapter data

Vanilla `UnitInitFromDefinition` (0x8017D3C) sets a loaded unit's class from vanilla's class table inline. Its literal
(0x8017DBC) points at `gClassData_NEW` (`Data/ClassData/Promotions.event`), so these classes, the Halberdier and every
other class can be placed in chapter unit data: a loaded unit has this repo's class data (bases, caps, growths, map
sprite) from the start, as it already had after a save or a suspend was reloaded (`GetClassData`). The boot smoke test
checks it (`boot: loaded units use gClassData_NEW`).

## Limitations

- The Master Seal's description is still vanilla's ("who has yet to change class").
- The Marshall's and the Trueblade's battle animations have their own colours on every team.

---

## Testing

`python3 Tools/cloud/smoketest.py --only t3promo` (`test_t3promo`): a Master Seal on a level-10 Swordmaster gives the
Trueblade at level 1 with EXP 0, the class's SMS id is 0x7D, her standing sprite draws (an OAM entry with her sprite's
tile, and the Trueblade's frame in VRAM there); a Swordmaster reaching level 20 is promoted, a Swordmaster F is not.

`--only t3promoall` (`test_t3promoall`): turn 1 ends with battle animations on, so the enemy phase's animated battles
leave `gEkrPopupEnded` set. A Master Seal then promotes Gilliam made a General, a General F, a Rogue, a Swordmaster and
a Sage; each is back on the map 2000 frames later, alive, a level-1 third tier with EXP 0 and no skill popup pending.
Last, Gilliam as a General at 19 with 99 EXP attacks with animations on and is promoted to the Marshall the same way.
Before the fix every case reset the game (the main loop read 0, two skills pending).

`--only autopromo` reads the level again 1500 frames after the automatic promotions: the map path showed 20 and 99.
