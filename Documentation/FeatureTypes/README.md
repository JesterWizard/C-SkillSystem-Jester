# Feature types: where to look and how to template each

Start every feature with [Common.md](Common.md) (anatomy, installer and `wizardry.event` placement, the designer-config toggle, text, and the docs and README entry). Then open the type doc that matches what the feature does (table below). A feature often spans several type docs: a menu row that stores a choice needs [Menus.md](Menus.md), a save chunk ([SaveData.md](SaveData.md)), RAM ([Memory.md](Memory.md)) and text. If the feature is really a skill (it has a SID and a skill name), go to [`../SkillTypes/README.md`](../SkillTypes/README.md) instead. Paths in this file are relative to the repo root.

## Pick a type

| If the feature... | Type doc | Main code / entry point |
|---|---|---|
| needs code at a fixed engine point (game start, chapter start, phase start, after an action, end of turn, unit load, copy, clear or kill, battle calc, AI decision), or replaces a vanilla function that has no hook point | [Hooks.md](Hooks.md) | hook lists in `data.event` files, for example `gPrePhaseFuncs` (`Kernel/Wizardry/PrePhaseHook/data.event:3`); `LYN_REPLACE_CHECK` plus `LynJump.event` for a replacement |
| adds a selectable command to the unit action menu, the map menu, the prep screen, the title/save Extras list, the Options screen, or a standalone menu | [Menus.md](Menus.md) | `gUnitActionMenuItemsRework` (`Data/UnitMenu/Source/UnitMenu.c:138`), `gMapMenuItems_NEW` (`Data/MapMenus/Usability.c:31`), `gPrepMenuTable` (`Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c:92`), `gGameOptions_NEW` (`Kernel/Wizardry/MenuOptions/MenuOptions.c:154`) |
| draws a window, stat screen page, help box or popup, or a proc-driven screen with its own sprites or palettes (not a selectable row) | [UI.md](UI.md) | `gStatScreenDrawPages` (`Kernel/Wizardry/StatScreen/data.event:10`), `gPopupComponents` (`Kernel/Wizardry/PopupRework/Source/PopupR.c:155`), `ProcScr_LanguageSelect` (`Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:353`) |
| adds an item, changes an item's use, target list or action, adds a staff, or sets a prep-screen or AI use for an item | [Items.md](Items.md) | `GetIERevamp` (`Kernel/Wizardry/ItemSys/IERevamp/Source/IER-core.c:6`) indexing `Kernel/Data/ItemSys/Source/IERevampTable.c`; item rows in `gItemData_New` (`Data/ItemSys/Source/Items.c`) |
| changes a battle rule or battle presentation without being a skill: pre-battle stats, a per-round cost, a cut-in, a banim step | [Battle.md](Battle.md) | `PreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:4`); `Banim_C07` (`Kernel/Wizardry/BanimHack/Source/C07.c:118`); `gPreMapAnimBattleRoundHooks` (`Kernel/Wizardry/MapAnimBattleHook/MapAnimBattleHook.event:12`) |
| adds a unit action (a turn command for player or CPU units) or an AI action | [UnitActions.md](UnitActions.md) | `gUnitActionTable` (`Kernel/Wizardry/UnitAction/Source/UnitActionData.c:13`); `gAiActionTable` (`Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c:4`) |
| adds a new event command (opcode), an ASMC (a C function that an event calls), or event flags | [Events.md](Events.md) | `gEventLoCmdTableRe` (`Kernel/Wizardry/EventRework/Source/EventTable.c:3`); `ASMC` (`Tools/FE-CLib-Mokha/include/EAstdlib.h:43`) |
| needs a global variable in kernel RAM, or code or data at a fixed ROM or IWRAM address | [Memory.md](Memory.md) | `_kernel_malloc` macros (`include/link/config-memmap.s:18`) |
| needs state that survives a save or suspend, or per-unit or per-chapter flags kept in SRAM | [SaveData.md](SaveData.md) | `gEmsSavChunks` (`Kernel/Wizardry/SaveData/data.event:39`), `gEmsSusChunks` (`Kernel/Wizardry/SaveData/data.event:70`) |
| draws pixels: a tile sheet, an LZ77 image, a palette, a portrait, a background, a map sprite animation | [Graphics.md](Graphics.md) | `Contents/Gfx/` (sheets and palettes), `Kernel/Wizardry/MapAnims/` (map sprite animations), `Kernel/Wizardry/Backgrounds_224_256_Colors/` (backgrounds) |
| must install on a clean FE8U (USA) ROM without the C kernel, as a FEBuilder "Insert EA" patch | [Standalone.md](Standalone.md) | `Standalone/<name>/` (26 folders, indexed in `Standalone/README.md`); `Commissions/<name>/` (1 folder) |

## Typical combinations

- **Boot language screen (LanguageSelect):** Common, UI (a proc-driven window), Graphics (the `.lz77` flag grid and its palette), Memory (`gKernelLanguage` through `_kernel_malloc2`), SaveData (the value lives in the global save header, not in a chunk). Each doc's worked examples cite `Kernel/Wizardry/LanguageSelect/`.
- **Vanilla replacement that changes a popup (PopupIconShine):** Common, Hooks (the `LynJump.event` and `LYN_REPLACE_CHECK`), UI (the popup icon proc).
- **Standalone menu opened from an event (ForgeMenu):** Common, Menus (the standalone-menu section), Events (the `CallForgeMenuASMC` ASMC), UI (its help box and sub-proc).
- **Chapter-scoped flag (PlayStExpa):** Common, Memory (`_kernel_malloc`), SaveData (both save tables; a `ChapterInitHooks` entry, `Kernel/Wizardry/ChapterInitHook/data.event:10`, clears it), Hooks (the chapter-start list).
- **New staff or usable item (Vulnerary, Slow staff):** Common and Items. Add Battle only for a new battle-side weapon effect (Items.md Gotchas, `WPN_EFFECT_*`).

## Finding how an existing feature was built

Run these from the repo root.

```bash
git log --oneline -- Kernel/Wizardry/PopupIconShine
git show --stat 0e82892cf
```

The first command lists the commits that touched the folder. The last line is the commit that created it (`0e82892cf`). The second lists every file that commit touched. For PopupIconShine it is 12 files:

- `Kernel/Wizardry/PopupIconShine/PopupIconShine_Installer.event`, `Kernel/Wizardry/PopupIconShine/Source/PopupIconShine.c`, `Source/LynJump.event`, `Source/PopupIconShine.lyn.event`
- `Kernel/Wizardry/wizardry.event` (one include line)
- `Data/DesignerConfig/designer-config.c` and `designer-config.lyn.event` (the toggle value)
- `include/kernel/kernel-lib.h` (the toggle field)
- `Documentation/Features/PopupItemIconShine.md`, `Documentation/Features/SystemConfig.md` (the toggle row), `Documentation/Gifs/Popup_Item_Icon_Shine.gif`, `README.md` (the bullet)

The doc name does not always match the folder: the folder is `PopupIconShine`, the doc is `Documentation/Features/PopupItemIconShine.md`. Search for the feature by its folder name and its doc name.

Then search the sources, excluding the stale copies under `.claude/`:

```bash
grep -rn --exclude-dir=.claude "PopupIconShine" Kernel Data include Contents Documentation README.md
```

Shell caveat: in some shells `grep` is a wrapper that skips gitignored files. Use `/bin/grep -rn ...` to include generated files such as `Contents/Gfx/GfxDefs.h`.

## Glossary

Definitions are cited from source. `path:line` is where the thing is defined or declared.

- **Installer (`*_Installer.event`)** — the one file per feature folder that `Kernel/Wizardry/wizardry.event` includes. Example: `Kernel/Wizardry/PopupIconShine/PopupIconShine_Installer.event`, included at `Kernel/Wizardry/wizardry.event:161`. Template and guard form in Common.md ("Installer template").
- **`WIZARDRY_DATA` / `WIZARDRY_TEXT` / `WIZARDRY_CUSTOM`** — the three blocks of `Kernel/Wizardry/wizardry.event`. `WIZARDRY_DATA` (`:1`) holds hook lists; `WIZARDRY_TEXT` (`:41`) is kernel `.text`; `WIZARDRY_CUSTOM` (`:129`) is the block for new features. Each is defined by the file that includes it: `Kernel/kernel.event:22` and `:36`, `main.event:60`.
- **FreeSpaceDEMO** — `include/configs/config-memmap.h:27` (`0x1000000`). `main.event:57` sets `ORG FreeSpaceDEMO` in front of the `WIZARDRY_CUSTOM` include (`main.event:59-62`). The standalone installers use their own space, `FE8U_StandaloneFreeSpace` = `$1000000` (27 installers, Standalone.md, Gotchas).
- **Kernel text section** — `KernelSpace_Text` (`0x0E8414`) and `KernelSpace_TextSize` (`0x7C000`) at `include/configs/config-memmap.h:15-16`. The assert is `Kernel/kernel.event:41`. It is nearly full (root CLAUDE.md, Pitfalls).
- **`.lyn.event`** — the Event Assembler file that `lyn` makes from a `.c`. Rule at `makefile:211-217`. It is committed, and some copies have hand edits (root CLAUDE.md).
- **`LynJump.event` / `LYN_REPLACE_CHECK`** — `LYN_REPLACE_CHECK(Func)` at `include/common-chax.h:26` keeps a replaced function in the build. `LynJump.event` is a `PUSH` / `ORG` / `POIN` / `POP` block that redirects a vanilla address to `POIN`'s function. Example: `Kernel/Wizardry/PopupIconShine/Source/LynJump.event:2` (`ORG $11254`).
- **`Preload/Reloc.event` vs `Kernel/Preload/KernelReloc.event`** — both hold `gp*` pointer words that C reads. `Preload/Reloc.event` points at `Data/` tables (`gpKernelDesignerConfig`, `:45`). `Kernel/Preload/KernelReloc.event` points at `Kernel/Wizardry/` lists (`gpPostActionFuncs`, `:15`). Rule in Common.md ("How kernel code reaches a Data table").
- **Hook list / `HookProc_Main`** — a `POIN` list in a `data.event`, ended by `WORD 0` (Hooks.md, Gotchas). Example: `gPrePhaseFuncs:` at `Kernel/Wizardry/PrePhaseHook/data.event:3`. The walker is `HookProc_Main` (`Kernel/Wizardry/KernelLib/Source/hook-proc.c:52`).
- **Designer config (`KernelDesigerConfig`, `gpKernelDesignerConfig`)** — the runtime toggle struct, spelled with the typo. Struct at `include/kernel/kernel-lib.h:8`; pointer at `:232`. Values in `Data/DesignerConfig/designer-config.c:5`. New fields go at the end of the struct (Common.md, "Does field order matter?").
- **Battle designer config (`KernelBattleDesignerConfig`, `gpKernelBattleDesignerConfig`)** — battle numbers, not on/off toggles. Values in `Data/DesignerConfig/battle-designer-config.c:4`; pointer at `include/kernel/kernel-lib.h:233` and `Preload/Reloc.event:134`.
- **`configs.h`** — `include/configs/configs.h`. `#define CONFIG_<NAME>` is a compile-time switch, read by both C and `.event` files. Its line 4 says these stay build-time because they are used by event and asm files. Gate with `#ifdef`, not `#if` (Common.md, "Compile-time switch").
- **`_kernel_malloc` / `_kernel_malloc2` / `_kernel_malloc_demo`** — RAM allocation macros at `include/link/config-memmap.s:18`, `:36`, `:27`. `_kernel_malloc` takes from the top-down pool (the most crowded). `_kernel_malloc2` takes from the bottom-up pool. `_kernel_malloc_demo` is the demo pool, and nothing in the tree uses it (Memory.md).
- **EMS chunk (`EmsChunk`, `gEmsSavChunks`)** — one save-data row. The macro is `Kernel/Wizardry/SaveData/data.event:18-19` (`EmsChunk(size, save, load[, idx])`). The SAV table is `gEmsSavChunks` (`:39`), the SUS table is `gEmsSusChunks` (`:70`). Order in the table is the layout (SaveData.md).
- **PlayStExpa (`gPlayStExpa`)** — a 128-bit chapter flag field. `extern u32 gPlayStExpa[4];` at `include/kernel/playst-expa.h:45`; `PlayStExpa_SetBit` / `PlayStExpa_CheckBit` / `PlayStExpa_ClearBit` at `:47-49`. Cleared at each chapter start (`Kernel/Wizardry/ChapterInitHook/data.event:10`).
- **UnitExpa / UES** — a per-unit 16-bit flag field. `SetBitUES` is declared at `include/kernel/unit-expa.h:67`; code is in `Kernel/Wizardry/UnitExpa/Source/UES.c`. Its bits do not survive a normal save (SaveData.md, Gotchas).
- **BWL** — the Battle/Win/Loss table. `Kernel/Wizardry/BwlRework/readme.md:1-3` names the table and says the rework also moves unit support data into it. Its save row is `EMS_CHUNK_BWLENTRIES` (`Kernel/Wizardry/SaveData/data.event:42`).
- **Proc (`Proc_Start`, `Proc_StartBlocking`)** — an engine task with a parent. `Proc_Start` at `Tools/FE-CLib-Mokha/include/proc.h:136`; `Proc_StartBlocking` at `:137`. A blocking child stops its parent (UI.md, Gotchas).
- **`MenuItemDef` / `MENU_ENABLED`** — one row of a menu. `struct MenuItemDef` at `Tools/FE-CLib-Mokha/include/uimenu.h:27`; its `isAvailable` callback at `:34` returns `MENU_ENABLED`, `MENU_DISABLED` or `MENU_NOTSHOWN` (enum at `:99-101`).
- **IER (item effect revamp)** — `ItemData::useEffectId` selects a row of `IERevampTable`. `IER_IS_VALID` at `include/kernel/item-sys.h:140`; lookup `GetIERevamp` at `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-core.c:6` (Items.md).
- **Unit action table (`gUnitActionTable`)** — one function per unit-action id, sized by `CONFIG_UNIT_ACTION_AMT` (`include/configs/configs.h:20`). Declared at `Kernel/Wizardry/UnitAction/Source/UnitActionData.c:13`; run by `ApplyUnitAction` (`Kernel/Wizardry/UnitAction/Source/UnitAction.c:36`).
- **AI action table (`gAiActionTable`)** — one row per AI action, sized by `CONFIG_AI_ACTION_AMT` (`include/configs/configs.h:15`). Declared at `Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c:4`; run by `CpPerform_PerformAction` (`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c:6`).
- **ASMC** — `ASMC(Func)` is `EvtAsmCall` (`Tools/FE-CLib-Mokha/include/EAstdlib.h:43`). An event calls a global C function by address. Keep the function non-static (Events.md, Gotchas).
- **Event command (`InjectEventLoCmd`)** — macro at `include/kernel/CustomDefinitions.event:26`. It writes a routine pointer into `gEventLoCmdTableRe` at `index * 4`. The table has `0x80` entries (`Kernel/Wizardry/EventRework/Source/EventTable.c:3`). Example: `Kernel/Wizardry/SkillSys/SkillSys.event:38`.
- **Standalone patch** — a FEBuilder "Insert EA" folder under `Standalone/` (26) or `Commissions/` (1). It installs without the kernel. Index and contract in `Standalone/README.md` (Standalone.md).
- **`.lz77` / `.4bpp` / `.gbapal`** — image files that grit builds from a PNG. Pattern rules at `makefile:279` (`%.4bpp`), `:284` (`%.gbapal`) and `:289` (`%.lz77`) (Graphics.md).
- **`GfxDefs.h`** — generated by `Contents/Gfx/makefile:10` through `gfx-installer.sh`. It is gitignored (`Contents/Gfx/.gitignore:1`). Edit the PNG, not the header (Graphics.md).

## Known stale docs and hazards

Do not copy from these without checking the paths against the tree. Each item is one the type docs report as found.

- `Documentation/Features/Save.md` is stale. Its SRAM numbers are out of date, and its claim that the kernel panics on save overflow is not in the code (SaveData.md, Gotchas).
- `Documentation/Memory.md` is stale. Its demo size `0x0C40` (line 52) should be `0xEB0` (`include/link/config-memmap.s:23-24`); its `ARM_UnitList` address is also wrong (Memory.md, Gotchas).
- `Documentation/Features/UnlimitedPrepMenuOptions.md` says the prep menu is unlimited. The prep table is sized for 12 rows, and call order (not table order) decides the display (Menus.md, Gotchas).
- `Patches/PATCH_DesignerConfig.txt` is out of sync with `KernelDesigerConfig`. Its last field is `B169` with `DATASIZE=170`, and it is hand-kept (Common.md, "Does field order matter?").
- `Documentation/Features/Halfbodies.md` says `half_body_portraits` defaults to `true`. `Data/DesignerConfig/designer-config.c:71` sets it to `false`. The doc follows the code (Graphics.md).
- `Documentation/Features/PopupR.md` is a three-line stub about the skill popup stack. Use UI.md (UI.md, Gotchas).
- RAM alignment is not checked or padded. The macros only move a pointer, and `_kernel_malloc_align4_pad` is commented out at `include/link/config-memmap.s:212`. Pair odd sizes (Memory.md, Gotchas).
- Save order is layout. Inserting or resizing a row moves every later chunk, so old saves load shifted data (SaveData.md, Gotchas; `Kernel/Wizardry/SaveData/Source/ModularSaveCore.c:204-212`, found by reading the code, not tested on a save).
- A unit-action id at or above `0x30` sets `US_HAS_MOVED` and does nothing else (UnitActions.md, Gotchas; `Kernel/Wizardry/UnitAction/Source/UnitAction.c:41`).
- An unknown event sub-op calls `hang()` (Events.md, Gotchas; `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c:221-223`).
- Kernel-only builds drop the assets. Only `main.event:59-62` includes `Contents/` and defines `WIZARDRY_CUSTOM` (Graphics.md, Gotchas).
