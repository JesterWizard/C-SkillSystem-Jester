# Max Simultaneous Support Conversations

Raises (or lowers) how many support ranks a unit can hold at the same time. A unit's total is the sum of its ranks with every partner (C = 1, B = 2, A = 3). Vanilla caps it at **5**, for example one A and one B. This patch sets the cap to **`MAX_SUPPORTS`** (default **10**, the C Skill System default).

With the default settings the new cap applies everywhere vanilla checks the old one:

| Where | Vanilla | With this patch |
|-------|---------|-----------------|
| Map **Support** command (`CanUnitSupportNow`, both units) | hidden when either unit has 5 | hidden when either unit has `MAX_SUPPORTS` |
| Support EXP gained at turn start (`ProcessTurnSupportExp`, both units) | stops at 5 | stops at `MAX_SUPPORTS` |
| Stat screen support list (green when full) | 5 | `MAX_SUPPORTS` |
| Preparations **Support** screen: "full" state of the unit list | 5 | `MAX_SUPPORTS` |
| Preparations **Support** screen: remaining-supports number | `5 - total` | `MAX_SUPPORTS - total` |

### Difference from the integrated C Skill System

In the integrated kernel the `max_simultaneous_support_conversations` flag only changes the last row, the "remaining" number on the preparations Support screen (`InitSupportSubScreenRemainingSupports`). The real limit stays at 5 there, because the kernel's `CanUnitSupportNow` and vanilla's support EXP code still use the constant `MAX_SIMULTANEOUS_SUPPORT_COUNT = 5`. So the screen can say 5 supports are left while the unit can't support anymore.

This package applies the value to the real limit as well, so the screen and the game agree. To get the integrated behaviour exactly (screen number only), uncomment `#define MAX_SUPPORTS_SCREEN_ONLY` in `Installer.event`.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: one immediate byte in each of 7 vanilla instructions (see below)
- Free space: none

## Settings

Edit the top of `Installer.event`:

| Define | Default | Meaning |
|--------|---------|---------|
| `MAX_SUPPORTS` | `10` | Support-rank cap, 1–255 |
| `MAX_SUPPORTS_SCREEN_ONLY` | not defined | If defined, only the prep-screen "remaining" number changes (integrated behaviour) |

## Build

There is nothing to build: the patch only changes instruction immediates from `Installer.event`.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `max_simultaneous_support_conversations` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-max-supports.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | Settings and the 7 byte patches |

## Hooks & Free Space

| Function | Instruction | `ORG` | New byte |
|----------|-------------|-------|----------|
| `InitSupportSubScreenRemainingSupports` | `movs r1, #5` at `0x080A2096` | `$A2096` | `MAX_SUPPORTS` |
| `sub_80A0F6C` (prep Support list state) | `cmp r0, #5` at `0x080A0F82` | `$A0F82` | `MAX_SUPPORTS` |
| `DisplaySupportList` (stat screen) | `cmp r0, #5` at `0x080876B4` | `$876B4` | `MAX_SUPPORTS` |
| `CanUnitSupportNow` (unit) | `cmp r0, #4` at `0x0802833E` | `$2833E` | `MAX_SUPPORTS - 1` |
| `CanUnitSupportNow` (partner) | `cmp r0, #4` at `0x0802834E` | `$2834E` | `MAX_SUPPORTS - 1` |
| `ProcessTurnSupportExp` (unit) | `cmp r0, #4` at `0x08028476` | `$28476` | `MAX_SUPPORTS - 1` |
| `ProcessTurnSupportExp` (partner) | `cmp r0, #4` at `0x08028508` | `$28508` | `MAX_SUPPORTS - 1` |

With `MAX_SUPPORTS_SCREEN_ONLY` only the first row is written.

**Free space:** none. No RAM. No save-format change: support EXP is stored per partner exactly as in vanilla.

## Conflicts

- Any patch that replaces `CanUnitSupportNow`, `ProcessTurnSupportExp`, `DisplaySupportList`, `sub_80A0F6C` or `InitSupportSubScreenRemainingSupports`, including the full C Skill System (its BWL support rework has its own `CanUnitSupportNow`). A replacement function makes the byte patch for that site have no effect; EA's `PROTECT` only reports overlaps with patches that also use `PROTECT`.
- `Standalone/dynamic_weapon_slots` hooks `DisplayWeaponExp` right after `DisplaySupportList`; the ranges do not overlap, and both can be installed.

## Limitations

- Every support conversation still only goes up to A, and each pair still has one A at most. The cap only limits how many ranks a unit can collect in total.
- Values above 21 have no effect in vanilla data: no character has more than 7 partners (7 × A = 21).

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
