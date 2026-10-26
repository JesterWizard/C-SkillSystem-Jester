# Class Growths

---

## 📑 Index
- [Introduction](#introduction)
- [Modes](#modes)
- [Previewing Growth Changes](#previewing-growth-changes)
- [Code Locations](#code-locations)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Class growths make a unit's class part of its growth rates, Fates-style. Promoting or reclassing therefore changes how the unit grows from then on, not just its stats on the day, which gives each promotion branch a second thing to weigh up.

The system is gated behind `gpKernelDesignerConfig->class_growths`. With it at `0`, growths are exactly the personal growths they were before.

The class growths used are the `growthHP` ... `growthLck` fields of each class in [`ClassData.c`](../../Data/ClassData/ClassData.c), plus the class magic growth from the Str/Mag split table (`gpMagicJInfos[class].growth`).

## ⚙️ Modes

| `class_growths` | Base growth used for level-ups | Notes |
|-----------------|--------------------------------|-------|
| `0` | personal | Off. Vanilla behaviour. |
| `1` | personal + class | Fates-style. Intended for class tables whose growths are written as modifiers (e.g. `+10`, `-5`). |
| `2` | (personal + class) / 2 | Works with vanilla-scale class growths (this repo's class table has entries like HP `90` / Str `45`) without doubling a unit's growths. |

The merged value replaces the personal growth at the very start of the growth chain, so everything that already adds to growths (skills such as Blossom or Aptitude, Prestige, the `gJobGrowthList` table, status effects) still applies on top. Every caller of `GetUnit*Growth` sees the result: level-ups, the timing level-up minigame, realistic autolevels and the stat screen.

On the stat screen with `stat_screen_growths = 2`, the blue base number includes the class part and the green bonus is only what skills and other effects add on top.

Class growths apply to every unit, so bosses that autolevel from personal growths (`UnitAutolevelRealistic`) are affected too. Generic units still autolevel from their class growths alone, as in vanilla.

If `lvup_stat_points` replaces growths entirely, the previews below stay hidden, since growths do not drive level-ups in that mode.

## 🔍 Previewing Growth Changes

**Promotion screen.** The class description box at the bottom shows how the highlighted class would change each growth, for example:

```
HP +5%  Str +10%  Mag -5%  Skl +0%
Spd -5%  Lck +0%  Def +15%  Res -10%
```

Press **R** to swap the box between the growth changes and the vanilla class description. The preview opens on the growth changes every time the screen opens.

**Vesly reclass screen** (`vesly_reclass`). The two frames beside the battle animation show stat changes as before. Press **R** to swap them to growth changes, drawn in green like the stat screen's growth bonuses. Growth changes beyond ±99 are shown as ±99 because the frames only fit two digits.

The numbers are worked out by running the unit through the normal growth getters twice, once as it is and once with its class swapped for the highlighted one, so anything class-dependent (including `gJobGrowthList`) is part of the preview.

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config field | `class_growths` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Mode: `0` off, `1` add, `2` average. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Off by default. |
| Merge rule and base getters | `GetClassGrowthBase`, `GetUnitBase*Growth` in [`ClassGrowths.c`](../../Kernel/Wizardry/ClassGrowths/Source/ClassGrowths.c) | Personal growth merged with the class growth. |
| Growth getters | `GetUnit*Growth` in [`GrowthGetter.c`](../../Kernel/Wizardry/Lvup/Source/GrowthGetter.c) | Feed the base getters into the existing bonus chain. |
| Preview helpers | `GetClassChangeGrowthDiffs`, `gClassGrowthStatNames` in [`ClassGrowths.c`](../../Kernel/Wizardry/ClassGrowths/Source/ClassGrowths.c) | Per-stat growth change for a target class. |
| Promotion screen preview | [`ClassGrowthPreview.c`](../../Kernel/Wizardry/ClassGrowths/Source/ClassGrowthPreview.c) | Replaces `ClassChgMenuExec`, `ClassChgMenuItem_OnChange` and `sub_80CCF60` (`PromoSel_StartClassDesc`) to draw the preview and handle R. |
| Reclass screen preview | `ReclassDrawGrowthChanges`, `ReclassMenuSel_OnRPress` in [`C_Code.c`](../../Kernel/Wizardry/VeslyReclass/C_Code.c) | R toggles the stat frames to growth changes. |
| Stat screen | `ToggleUnitPageGrowth` in [`ToggleUnitPage.c`](../../Kernel/Wizardry/StatScreen/DrawUnitPage/Source/ToggleUnitPage.c) | Folds the class part into the base number. |
| Preview text buffer | `sClassGrowthPreviewText` in [`config-memmap.s`](../../include/link/config-memmap.s) | RAM for the description box string, which the talk system reads while it prints. |

The module lives in `FreeSpaceDEMO` (`WIZARDRY_CUSTOM` in `wizardry.event`) so it does not overflow kernel `.text`.

## 🐛 Limitations & Bugs

- The stat names in both previews are hard-coded English, the same as the Vesly reclass stat frames.
- `Patches/PATCH_DesignerConfig.txt` does not list `class_growths` yet. Its offsets were already behind the struct before this feature (it ends at `weapon_rank_skills`), so it needs a full resync rather than one extra line.

Please report any issues in the repository's Issues tab.
