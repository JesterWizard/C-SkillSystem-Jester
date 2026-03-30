# Menu Skill AI

<p align="center">
  <img src="../Gifs/Menu_Skills_AI.gif" alt="Menu Skills AI" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

This feature gives AI units a conservative fallback for menu skills.

The priority order stays the same:

1. Try to win the fight with the existing combat AI.
2. Try to heal or escape with the existing staff and movement logic.
3. If neither of those produces an action, try a menu skill that passes the normal usability check.

That means the AI becomes more competent with support skills without replacing the existing combat evaluator or staff logic.

## ✅ Usable Menu Skills

The AI fallback currently recognizes these menu skills:

### Self-target skills

- HealingFocus
- RallyDefense
- RallyLuck
- RallyMagic
- RallyMovement
- RallyResistance
- RallySkill
- RallyStrength
- RallySpectrum
- GoddessDance
- Stride
- Songstress
- DancePlus
- Transform
- Dismount
- Bide
- CoinFlip
- DoubleTime
- BravelyDefault
- GraceOfFire
- GraceOfWater
- Hide
- Fogger
- Reinforce
- Reroll
- BloodyAlchemy
- Gungnir
- Arise
- EmergencyExit
- EmergencyExitPlus

### Targeted skills

- LightRune
- Mine
- Summon
- ArdentSacrifice
- Sacrifice
- ReciprocalAid
- FocusEnergy
- AssignDecoy
- AssignDecoyPlus
- GorillaTactics
- SpellBlade
- Kamikaze
- Doppleganger
- DeathBlight
- Transcendence
- Persuade
- PersuadePlus
- WyvernCrash

---

## 🛠️ Plan

### Behavior

| Stage | Behavior | Why it exists |
|------|----------|---------------|
| 1 | Combat and kill checks | Keeps the best offensive choice ahead of everything else. |
| 2 | Healing and escape checks | Preserves the normal support and survival behavior. |
| 3 | Self-target menu skills | Lets the AI use any menu skill that resolves on the acting unit. |
| 4 | Targeted menu skills | Lets the AI use menu skills that build a valid target list. |

### Performance concerns

The main cost is extra AI-phase decision time.

The implementation keeps that cost bounded by design:

- It only scans the acting unit’s own skill list.
- It uses the same usability logic the player menu uses.

Even with those limits, the cost still scales with the number of AI units. If a map has many active enemies, the extra time adds up across the entire phase. That is why the feature is behind a designer-config switch.

### Future AI improvements

The current version chooses the first valid usable skill it finds. Smarter behavior could be added later by:

- Ranking skills by tactical value instead of taking the first valid one.
- Caching target lists during a single AI decision so the same local search is not rebuilt twice.
- Adding custom rules for specific targeted skills that need better target choice than "first legal target".

---

## 🗂️ Code Locations

The feature is gated behind `gpKernelDesignerConfig->menu_skill_ai_use` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c).

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config flag | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Adds the runtime boolean that gates the AI menu-skill fallback. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Leaves the feature disabled by default so existing projects keep their current behavior. |
| AI decision hook | `DecideScriptA`, `DecideScriptB`, `DecideHealOrEscape`, and `AiTryDoStaff` in [`AiOptimization.c`](../../Kernel/Wizardry/Core/AiHack/AiOptimization/Source/AiOptimization.c) | Calls the menu-skill fallback only after the normal higher-priority AI passes do not choose an action. |
| Menu-skill fallback | `AiTryDoMenuSkills` in [`MiscAiSkills.c`](../../Kernel/Wizardry/Misc/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c) | Scans the acting unit’s skills and applies the first usable menu skill it can execute. |
| Skill execution path | `AiAction_MenuSkill` in [`MiscAiSkills.c`](../../Kernel/Wizardry/Misc/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c) | Reuses `CONFIG_UNIT_ACTION_EXPA_ExecSkill` so the normal skill action pipeline performs the move. |
| Menu usability gate | `MenuSkills_Usability` in [`SkillMenu.c`](../../Kernel/Wizardry/Core/SkillSys/kernel/SkillMenu.c) | The AI still respects the same usability callbacks that the player menu uses. |

---

## 📝 TODO

- Add per-skill weighting so tactical support skills can outrank low-value ones.
- Add a small profiling pass for large enemy phases so the AI-time cost can be measured directly.
- Decide whether any specific menu skills should remain player-only even when the AI fallback is enabled.

---

## 🐛 Limitations & Bugs

- The AI still picks the first valid target in a target list, so it may not choose the best possible target.
- Enabling the feature adds extra work for every AI unit that reaches the fallback path, so large maps will feel the cost more than small ones.
- Some menu skills may still need dedicated heuristics later if they depend on context that cannot be captured by a basic target list.