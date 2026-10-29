# Talk Conversation EXP Reward

After an on-map **Support** conversation, the unit that used the Support command gains **10 EXP** (configurable). The usual map EXP bar is shown, and a level-up plays if the unit reaches 100 EXP.

Despite the flag's name, the integrated C Skill System only grants this reward at the end of on-map support conversations (`TalkEXPGain` is an `ASMC` in its replacement of `EventScr_MapSupportConversation`). Chapter **Talk** conversations, the Extras support viewer and base conversations give no EXP, in the kernel and here.

This matches the integrated C Skill System behaviour of `talk_conversation_exp_reward` (default `10`), but installs on **clean FE8U** without the kernel.

## Configuration

The EXP amount is the byte `gSupportTalkExpReward` in `Installer.event`:

```
gSupportTalkExpReward:
    BYTE 10 // EXP the unit who used the Support command gains after the conversation (0-100, 0 = off)
```

Edit it before inserting; no rebuild is needed.

The reward follows vanilla misc-action EXP rules (`BattleApplyMiscActionExpGains`): only player units gain it, and not at max level or while `PLAY_FLAG_EXTRA_MAP` is set (vanilla extra/trial maps).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `CallMapSupportEvent` at `0x0800D304` (installer ORG `$D304`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/SupportTalkExpReward.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/SupportTalkExpReward.c`:

```bash
make -C Standalone/talk_conversation_exp_reward
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `talk_conversation_exp_reward` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-support-exp.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, EXP setting, hook, and free-space placement |
| `Source/SupportTalkExpReward.c` | `CallMapSupportEvent` replacement, support event script, EXP `ASMC` and EXP-bar proc |
| `Source/SupportTalkExpReward.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `CallMapSupportEvent` (`0x0800D304`, only called by `StartSupportTalk`) | `$D304` | 8 bytes |

The replacement starts a copy of vanilla `EventScr_MapSupportConversation` (`0x08591FF0`, byte-identical: music, conversation, "Support Lv increased" popup) with one extra `ASMC SupportTalkExpGain` before its end. That function loads the active unit into `gBattleActor`, applies the EXP like vanilla misc-action EXP, and starts a small proc that waits for the event to end, then shows the vanilla map EXP bar (`MapAnim_DisplayExpBar`, with level-up) and writes level, EXP and stat gains back to the unit (`UpdateUnitFromBattle`, keeping the unit's live state bits).

**Free space:** `$1000000`, 0x1CC bytes including the 1-byte setting (body continues at `CURRENTOFFSET` after install). No RAM is used.

## Conflicts

- Any patch that replaces `CallMapSupportEvent` (`$D304`) or edits `EventScr_MapSupportConversation` (this patch no longer uses the vanilla script), including C Skill System `BwlRework` (`BwlSupport.c`).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site and the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- Only the unit that chose **Support** gains EXP, not its partner (same as the kernel).
- Dropped from the integrated replacement script: the `support_rewards` item gift (a separate kernel flag). The kernel script also drops the vanilla "Support Lv increased" popup; this patch keeps it, so the popup is shown before the EXP bar.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c`, `TalkEXPGain` / `AddExp_Event`, and `Kernel/Wizardry/BwlRework/source/BwlSupport.c`).
