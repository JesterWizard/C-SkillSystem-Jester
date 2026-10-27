# Save and suspend data

> Code: `Kernel/Wizardry/SaveData/`, `Kernel/Wizardry/ChapterInitHook/`, `Kernel/Wizardry/PlayStExpa/`, `Kernel/Wizardry/UnitExpa/`  ·  Entry points: `gEmsSavChunks` and `gEmsSusChunks` (`Kernel/Wizardry/SaveData/data.event:39`, `:70`), `gNewSaveHooks` (`data.event:119`), `ChapterInitHooks` (`Kernel/Wizardry/ChapterInitHook/data.event:3`)  ·  Existing docs: `Documentation/Features/Save.md` (stale, see Gotchas), [Memory.md](Memory.md), [EndTurn.md](../SkillTypes/EndTurn.md) (chapter-init section)

## What it covers

Persisting a feature's state in SRAM: a chunk in the normal save (SAV, written with a saved chapter) or in the suspend save (SUS, written at suspend points). It also covers the per-unit data that travels in the unit packs, the per-chapter flag bits, and what is reset at a new game and at each chapter start. Examples: the Snek high score (both tables, magic word), the ConvoyExpa item array (sizes set by hand), PlayStExpa flag bits, and LanguageSelect, which keeps its value in the global save header and not in a chunk.

## How the engine calls it

### Save and load flow

1. Vanilla save routines are replaced by C functions at their ROM addresses in `Kernel/Wizardry/SaveData/Source/LynJump.event` (`POIN` rows at `:23` WriteNewGameSave, `:29` WriteGameSave, `:35` ReadGameSave, `:41` WriteSuspendSave, `:47` ReadSuspendSave). The C is in `Kernel/Wizardry/SaveData/Source/ModularSaveCore.c`.
2. `WriteNewGameSave` (`ModularSaveCore.c:160`) sets up `gPlaySt`, sets `playthroughIdentifier` (`:180`), runs each entry of `gpNewSaveHooks` (`:185-186`), then calls `WriteGameSave` (`:189`).
3. `WriteGameSave` (`:193`) walks `gEmsSavChunks` in order. Each row calls `cur->save(dst + offset, cur->size)` and adds `cur->size` to `offset` (`:204-212`). After the table, `MSA_ExternalSaver` gets the rest of the slot (`:216`).
4. `ReadGameSave` (`:225`) calls `InitUnits()` (`:235`), then walks `gEmsSavChunks` calling `cur->load` (`:244`), then `MSA_ExternalLoader` (`:249`).
5. `WriteSuspendSave` (`:256`) walks `gEmsSusChunks` (`:299`) then `MSU_ExternalSaver` (`:304`). `ReadSuspendSave` (`:315`) mirrors this (`:322`, `:331`, `:336`). The suspend callers include `Kernel/Wizardry/PhaseSuspend/Source/PhaseSuspend.c:15`. Other callers were not traced.
6. Table format. `Kernel/Wizardry/SaveData/data.event:18-19` defines `EmsChunk(size, save, load[, idx])`. Each row becomes `SHORT size`, the two function pointers, and the index word. A table ends with `EmsChunkEnd` (`data.event:67`, `:116`). Slot sizes are in `gEmsSizes` (`data.event:14-16`): SAV `0x1940`, SUS `0x3150`. Slot offsets are in `gEmsOffsets` (`data.event:2-9`). The magic word is at `gSkillSysMagicOffset` = `0x7DE4` (`data.event:11-12`).
7. The tables sit in the WIZARDRY_DATA block (`Kernel/Wizardry/wizardry.event:19`). The functions they name can be in the CUSTOM block. Forward references work: the Snek rows (`data.event:48`, `:85`) name functions from `Kernel/Wizardry/Minigames/Snek/Snek.lyn.event`, included from `Snek_Installer.event` (`Kernel/Wizardry/wizardry.event:157`).

### Chapter start and new game

- New game: `gNewSaveHooks` (`data.event:119-125`) runs inside `WriteNewGameSave` before the first save (`ModularSaveCore.c:185-186`). Its entries include `NewGame_InitPlayStExpa` (`data.event:121`).
- Chapter start: `ChapterInitHooks` (`Kernel/Wizardry/ChapterInitHook/data.event:3-20`) is walked by `ChapterInitHook` (`Kernel/Wizardry/ChapterInitHook/Source/ChapterInit.c:7-16`) before the vanilla `UndeployEveryone()`. The skill-side details are in [EndTurn.md](../SkillTypes/EndTurn.md).

### Per-unit data

- The unit packs are chunks of their own. `struct EmsPackedSavUnit` is 0x2C bytes (`include/kernel/save-data.h:44`, `SIZE_OF_SAV_UNIT_PACK` at `:121`). `struct EmsPackedSusUnit` is 0x34 bytes (`:60`, `:122`). The ally packs are `MSA_SaveUnits` (`data.event:59`) and `MSU_SaveBlueUnits` (`data.event:95`). The packing is in `NewPackSaveUnit` (`Kernel/Wizardry/SaveData/Source/MsaFunc.c:80`) and `NewPackSuspandUnit` (`Kernel/Wizardry/SaveData/Source/MsuFunc.c:133`).
- Skill bytes. `struct Unit` has `supports[7]` (`Tools/FE-CLib-Mokha/include/types.h:442` gives `UNIT_SUPPORT_MAX_COUNT = 7`). Ally packs hold all 7 bytes in `pad.ally.skills[7]` (`save-data.h:80`). Enemy, NPC and fourth-allegiance packs hold only 3 in `pad.ai.skills[3]` (`save-data.h:99`). The rest go in separate chunks, `MSU_Save{Red,Green,Purple}UnitExtSkills` (`data.event:100-102`; `MsuFunc.c:401-466`).
- Unit-expa bits. `_u3A` and `_u3B` (`Tools/FE-CLib-Mokha/include/bmunit.h:173-174`) are 16 bits of per-unit flags. Only the SUS pack keeps them (`save-data.h:117-118`; copied at `MsuFunc.c:213-214` and `:281-282`). The SAV pack does not. Helpers: `SetBitUES`, `ClearBitUES`, `CheckBitUES` (`Kernel/Wizardry/UnitExpa/Source/UES.c:11-27`). The bit enum is `unit_expa_sus_bitfile_idx` (`include/kernel/unit-expa.h:12`). `ResetUnitsExpaSus` (`UES.c:40`) is a chapter-init entry (`ChapterInitHook/data.event:5`).
- BWL. `GetNewBwl(pid)` (`Kernel/Wizardry/BwlRework/source/BriefBwl.c:17`) indexes `gPidStatsData` by character ID. The entry struct is `include/kernel/bwl.h:7`. It is saved by a chunk with vanilla save and load addresses (`data.event:42` in SAV and `:75` in SUS), size `$0330`.
- The `DemoUnitExpa` pool in `Kernel/Wizardry/UnitExpa/Source/UnitExpa.c` is commented out. Do not use it.

### Per-playthrough and per-chapter flags

- `gPlayStExpa` is a 16-byte field (`include/link/config-memmap.s:91`) declared as `extern u32 gPlayStExpa[4];` (`include/kernel/playst-expa.h:45`). Bits are `PLAYSTEXPA_BIT_*` (`playst-expa.h:5-41`; 34 used; `PLAYSTEXPA_BIT_MAX = 128` at `:42`). It is saved as `EMS_SavePlayStExpa` in both tables (`data.event:58`, `:94`).
- Reset: `NewGame_InitPlayStExpa` (on a new game, `data.event:121`) and `ChapterInit_ResetPlayStExpa` (every chapter start, `ChapterInitHook/data.event:10`). Both `memset` the field to 0 (`Kernel/Wizardry/PlayStExpa/Source/PlayStExpa.c:5-13`). The bits are per chapter, not per playthrough, despite the "used" wording.
- Playthrough. `gPlaySt` is the chapter data struct (`Tools/FE-CLib-Mokha/include/types.h:208`). It is saved in chunk 1 (`MSA_SavePlaySt`). Its `playthroughIdentifier` is at `0x18` (`types.h:228`) and is set in `WriteNewGameSave` (`ModularSaveCore.c:180`).

### Global save header (not a chunk)

`struct GlobalSaveInfo` (`Tools/FE-CLib-Mokha/include/bmsave.h:48`) is read and written with `ReadGlobalSaveInfo` and `WriteGlobalSaveInfo` (`bmsave.h:444-445`). `LanguageSelect` keeps its choice in `unk10` (`bmsave.h:64`; `Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:84-118`). This header is not one of the EMS tables and is not reset at chapter start.

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/SaveData/data.event` | Both chunk tables (`gEmsSavChunks`, `gEmsSusChunks`), slot sizes, offsets, new-game hooks |
| `Kernel/Wizardry/SaveData/Source/ModularSaveCore.c` | Save and load engine, `WriteNewGameSave`, `WriteGameSave`, `ReadGameSave`, suspend pair |
| `Kernel/Wizardry/SaveData/Source/LynJump.event` | ROM redirects for the save routines |
| `Kernel/Wizardry/SaveData/Source/MsaFunc.c`, `MsuFunc.c` | Chunk functions for the normal save and the suspend save, and unit packing |
| `Kernel/Wizardry/SaveData/Source/SramVerify.c` | Skill-system magic check (`CONFIG_VERIFY_SKILLSYS_SRAM`) and pack size asserts |
| `Kernel/Wizardry/SaveData/Source/NewGameSaveHook.c` | Generic new-game hook (clears world map, dungeon, timer) |
| `Kernel/Wizardry/SaveData/SaveData.event` | Include list and the SRAM layout comment |
| `Kernel/Wizardry/SaveData/SaveDataExpansion/RelocMiscChunks.event` | Moves the vanilla SRAM tail (from `0x7DEC`) |
| `include/kernel/save-data.h` | `struct EmsChunk`, the two unit packs, `SIZE_OF_*`, external hook pointers |
| `Preload/Reloc.event:40-43` | `gpMsa_Saver` and related external hooks (default `WORD 0`) |
| `Kernel/Preload/KernelReloc.event:16` | `gpNewSaveHooks` pointer |
| `include/configs/configs.h` | `CONFIG_UNIT_AMT_*`, `CONFIG_INSTALL_CONVOYEXPA_AMT`, `CONFIG_VERIFY_SKILLSYS_SRAM` |
| `Tools/FE-PyTools/save-memory-usage.py` | SRAM budget report (the tool `make savecheck` runs) |
| `Kernel/Wizardry/ChapterInitHook/data.event` | `ChapterInitHooks` list |
| `Kernel/Wizardry/PlayStExpa/Source/PlayStExpa.c` | Bit field reset and save |
| `Kernel/Wizardry/UnitExpa/Source/UES.c` | Unit-expa bit helpers |
| `Kernel/Wizardry/BwlRework/source/BriefBwl.c` | BWL entry lookup |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Decide the lifetime. Normal save (SAV) is written with a saved chapter. Suspend (SUS) is rewritten at each suspend point. A value can be in one, both or neither.
3. Allocate the RAM with `_kernel_malloc` and declare it `extern` ([Memory.md](Memory.md)).
4. Write the pair `void X_SaveState(u8 *dst, const u32 size)` and `void X_LoadState(u8 *src, const u32 size)`. Use `WriteAndVerifySramFast` and `ReadSramFast`. Add `Assert(size >= sizeof(...))`. Add a magic word when old or blank SRAM must not be trusted (`Snek_LoadHighScore`, `Kernel/Wizardry/Minigames/Snek/Snek.c:17-27`).
5. Add `EmsChunk($XXXX, X_SaveState, X_LoadState)` to `gEmsSavChunks` and/or `gEmsSusChunks` in `Kernel/Wizardry/SaveData/data.event`, before `EmsChunkEnd`. Append. Never insert or resize a row in the middle (Gotchas).
6. Installer. Add `#include "Source/X.lyn.event"` to the feature's installer, and add the installer's include to the `WIZARDRY_CUSTOM` block of `Kernel/Wizardry/wizardry.event` (`:129`).
7. If the value starts fresh on a new game, add its reset function to `gNewSaveHooks` (`data.event:119`). If it resets each chapter, add it to `ChapterInitHooks` before `WORD 0` (`ChapterInitHook/data.event`).
8. Run `make savecheck` and `make ramcheck`. At writing, SUS had only 164 bytes free (Gotchas).

## Template

Adapted from the Snek high score (`Kernel/Wizardry/Minigames/Snek/Snek.c:7-27`; table rows `Kernel/Wizardry/SaveData/data.event:48` and `:85`). It needs the RAM from [Memory.md](Memory.md) (`_kernel_malloc gExampleState, 2` and `extern u16 gExampleState;`).

```c
/* Kernel/Wizardry/Example/Source/Example.c */
#include "common-chax.h"
#include "save-data.h"
#include "example.h"

#define EXAMPLE_SAVE_MAGIC 0x4558

void Example_SaveState(u8 *dst, const u32 size)
{
	u16 buffer[2];

	Assert(size >= sizeof(buffer));
	buffer[0] = gExampleState;
	buffer[1] = EXAMPLE_SAVE_MAGIC;
	WriteAndVerifySramFast(buffer, dst, sizeof(buffer));
}

void Example_LoadState(u8 *src, const u32 size)
{
	u16 buffer[2];

	Assert(size >= sizeof(buffer));
	ReadSramFast(src, buffer, sizeof(buffer));

	if (buffer[1] == EXAMPLE_SAVE_MAGIC)
		gExampleState = buffer[0];
	else
		gExampleState = 0;
}
```

Table rows, in `Kernel/Wizardry/SaveData/data.event`, before `EmsChunkEnd` in each table:

```
    EmsChunk($0004, Example_SaveState, Example_LoadState)
```

Installer, `Kernel/Wizardry/<Feature>/<Feature>_Installer.event` (here `Example`):

```
#ifndef EXAMPLE_INSTALLED
#define EXAMPLE_INSTALLED
#include "Source/Example.lyn.event"
#endif
```

Include line, in the `WIZARDRY_CUSTOM` block of `Kernel/Wizardry/wizardry.event`:

```
#include "Example/Example_Installer.event"
```

## Worked examples

- `Kernel/Wizardry/Minigames/Snek/Snek.c:7-27`: the smallest real chunk. Two `u16` (score and magic), validated on load. Rows `data.event:48` and `:85`.
- `Kernel/Wizardry/ConvoyExpa/Source/ConvoyExpa.c:12-23`: a 400-byte array saved with `Assert(size <= sizeof(...))` and `memset` on load. Its size is hand-set in four places that must agree: row `data.event:50` and `:86` (`$0190`), RAM `_kernel_malloc sExpaConvoyItemArray, 2 * 200` (`include/link/config-memmap.s:84`), and `CONFIG_INSTALL_CONVOYEXPA_AMT` (`include/configs/configs.h:10`, read by `Kernel/Wizardry/ConvoyExpa/ConvoyExpa.event:10-11`). The extern bound in `ConvoyExpa.c:7` is `0x300`, but the runtime limit is `sExpaConvoyItemAmount`.
- `Kernel/Wizardry/PlayStExpa/Source/PlayStExpa.c:5-22`: a bit field saved in both tables, cleared on a new game and on every chapter start.
- `Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:84-118`: the choice is cached in RAM (`gKernelLanguage`) and stored in the global save header. No EMS row.

## Gotchas

- **Order is layout.** Each chunk's offset is the sum of the sizes before it (`WriteGameSave`, `ModularSaveCore.c:204-212`; `CalcChunkOffset_Sav`, `:29-47`). Inserting or resizing a row moves every later chunk, so old saves load shifted data. Found by reading the code. Not tested on a real save.
- **The only version check is a magic.** `CONFIG_VERIFY_SKILLSYS_SRAM` (`include/configs/configs.h:8`, `SaveData.event:29`) compiles in `SramVerify.c`, which compares 8 bytes `FESKILL\0` at `0x7DE4` (`SramVerify.c:4-21`). On a mismatch it does not reject the save. `EraseInvalidSaveData` calls `WipeSkillSysSRAM`, which runs `InitGlobalSaveInfodata` and rewrites the magic (`SramVerify.c:23-31`, `:34-41`). The save slots themselves are not cleared by this code (what `InitGlobalSaveInfodata` touches was not traced). The magic does not change when the layout changes, so a layout change is not detected.
- **Size limits are not checked at build time.** The slot sizes are `0x1940` and `0x3150` (`data.event:14-16`). Nothing sums the rows at build time. The overflow test runs at the top of each loop iteration (`ModularSaveCore.c:205`, `:238`, `:293`, `:325`), so a row that crosses the end is written before the test fires. The boot-time check `GameInit_DetectEmsChunks` is registered (`Kernel/Wizardry/GameInitHook/data.event:13`), but its body is all comments (`ModularSaveCore.c:342-377`). `Documentation/Features/Save.md` says the kernel panics at game start on overflow. That is not in the code, and its SRAM numbers (`0x3000`, `0x19B0`) are out of date.
- **Hand-set sizes.** The BWL row is `$0330` and the convoy row is `$0190`, both written as literals. The unit rows use `CONFIG_UNIT_AMT_*` (`data.event:59`, `:95`, `:100-102`), so they follow `include/configs/configs.h:61-72`.
- **A size-0 row is still called.** `WriteGameSave` calls every row (`ModularSaveCore.c:211`). `MSA_SaveGaidenMagicUses` is an empty function, and `MSA_LoadGaidenMagicUses` resets the uses on every load (`Kernel/Wizardry/GaidenMagic/Source/ThreeHouses.c:226-231`; row `data.event:66`). A zero size is a no-op slot, not a removal.
- **SUS is nearly full.** `Tools/FE-PyTools/save-memory-usage.py` at writing: NormalSave 5103 of 6464 bytes used, SuspendSave 12460 of 12624 (164 free). A small row fits. The SRAM layout is fixed numbers: the first save slot is at `0x3224`, which is `0xD4 + 0x3150` (`data.event:3-5`, `:15-16`). Growing SUS therefore also moves every save-slot offset in `gEmsOffsets`.
- **Unit packs are asserted.** `EraseInvalidSaveData` asserts `sizeof` for both packs (`SramVerify.c:48-49`). Adding a field changes `SIZE_OF_SAV_UNIT_PACK` or `SIZE_OF_SUS_UNIT_PACK` (`save-data.h:121-122`) and the unit rows (`data.event:59`, `:95`).
- **The unit-expa bit field is 16 bits.** `SetBitUES`, `ClearBitUES` and `CheckBitUES` assert `bit < 16` (`UES.c:7`, `:13`, `:19`, `:25`). The enum has 17 entries when all listed skills exist (all 16 SIDs are in `include/constants/skills-equip.enum.txt`). `UES_BIT_TONIC_USED` is index 16 (`include/kernel/unit-expa.h:62`) and has no users in the tree. Do not use it. Widening the field changes the SUS pack (`save-data.h:117-118`).
- **Unit-expa bits do not survive a normal save.** The SAV pack has no `_u3A` (`save-data.h:44-58`). A bit that must survive a normal save needs another home (unverified where).
- **PlayStExpa is per chapter.** It is cleared at every chapter start (`ChapterInitHook/data.event:10`), so a flag meant to last a whole playthrough does not belong there.
- **Stay out of the vanilla tail.** `RelocMiscChunks.event` moves the vanilla bonus, rank and link-arena data to the top of SRAM (from `0x7DEC`), and the skill magic is at `0x7DE4`. A new chunk does not go there.
- **Chunk sizes can be odd.** Rows `$0001` and `$0019` exist (`data.event:41`, `:53` in the SAV table). The even-address rule in [Memory.md](Memory.md) applies to RAM only.
- **`gpMsa_Saver` and related hooks** (`Preload/Reloc.event:40-43`) default to `WORD 0`. `MSA_ExternalSaver` and `MSU_ExternalSaver` call them for the bytes after the table (`Kernel/Wizardry/SaveData/Source/ExternalLoader.c:4-23`). A feature that uses them takes the tail of the slot.
