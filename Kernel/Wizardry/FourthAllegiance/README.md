# FE8U-Fourth-Allegiance
 A hack that adds a fourth allegiance to FE8U, Based on the 4th army used by the link arena. Note that this hack is incompatible with the skill system family of engine hacks.
# How to Install

## Non-buildfile users
 Requires [Event Assembler](https://feuniverse.us/t/event-assembler/1749).
 You will need to install this engine hack into your ROM's Free Space. To do that, add ORG (YourFreeSpaceOffset) at the top of MasterAsmInstaller.event. 
 Then, use event assembler to assemble MasterAsmInstaller.event into your rom, and you should be good to go.
 
## Buildfile users
 Simply include MasterAsmInstaller.event into your buildfile, and everything should be good to go
 
# In this buildfile

 - There are **10 purple units** (`CONFIG_UNIT_AMT_FOURTH`, `include/configs/configs.h`): unit indexes 0xC1..0xCA. The
   installer's `PurpleUnitCount` (the engine's count, `gPhaseUnitCountTable`) is that same number, and the build fails
   if it is above 10.
 - 0xC1..0xC5 are vanilla's five purple slots (0x0202E36C). 0xC6..0xCA are new slots at 0x02026E30 (0x48 bytes each,
   `NewUnitRAMPointerSetter.event`); the lookup entries after them are 0. The kernel's RAM region starts right after
   them (`FreeRamSpaceTop`, 0x02026FC8, in `include/link/config-memmap.s`), and assembling that file fails if the
   region would grow into them.
 - Before this, the count was 16 and the lookup table had 15 new slots: InitUnits (a new game, a load) cleared the last
   few, which lay in the kernel's RAM (the class growth preview text and text engine state), and units 11 to 16 were
   never saved.
 - To raise the count: add pointers in `NewUnitRAMPointerSetter.event`, move `FreeRamSpaceTop` up by 0x48 a unit, and
   raise the suspend save's purple chunks (`Kernel/Wizardry/SaveData/data.event`) and `STAT_DEBUFF_SLOTS_FOURTH`
   (`include/kernel/debuff.h`).

# Usage

## Adding 4th allegiance units
 To add units of the 4th allegiance, simply set the faction in your unit definition to 3. In FEBuilder, that should appear as "Defeat".

## Setting Faction Relations per chapter
 Faction relations are handled via a bitmap. The short at +0x26 in the chapter data table is said bitmap. (Under "Enemy Phase BGM Flag 4" for FEBuilder users.)
 This hack automatically sets all chapters to have the vanilla relations for Blue, Red and Green units, and has Purple units be hostile to everyone.
 There are 4 bitmaps, in the order Blue-Green-Red-Purple; one per faction, and then each bit sets if you are allied or enemy to a faction, in the same order. Edit that bitmap to set your relations however you'd like.
 
## Setting Map music for the fourth faction
 Short 0x1C in the chapter data table is map music for the fourth phase("Player Phase BGM 2" for FEBuilder users), while short 0x1E is map music for the fourth phase when flag 4 is set("Enemy Phase BGM 2" for FEBuilder users).
 