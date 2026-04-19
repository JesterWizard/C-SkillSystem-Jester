# What Is CHAX

CHAX started from [StanHash/FE-CHAX](https://github.com/StanHash/FE-CHAX) and focuses on writing engine hacks in C instead of ASM wherever practical. Early work centered on the `elf2ea` pipeline and [lyn](https://feuniverse.us/t/ea-asm-tool-lyn-elf2ea-if-you-will/2986), but the project has grown alongside [FE8 decomp](https://github.com/FireEmblemUniverse/fireemblem8u) and tooling such as [ea-dep](https://github.com/StanHash/ea-dep).

In practice, CHAX is the foundation that lets this project compile a large set of engine changes together while keeping data, symbols, and hooks easier to maintain.

## Player-Facing Goals

- Up to **7** equipable skills for ally units, with skill selection in the prep screen.
- FE Engage style combo attacks that let nearby allies join combat.
- FE Three Houses style Combat Arts that trade durability for stronger attacks or special effects.
- Additional AOE systems and UI extensions.

## Developer-Facing Goals

- Build the full project at once with a single `make` command.
- Share the same header and symbol model with decomp work.
- Prefer C hacks over ASM hacks when possible.
- Provide a practical debugging toolkit.
- Use a faster `SkillTester()` design via [SkillList.c](../Kernel/Wizardry/Core/SkillSys/kernel/SkillList.c).
- Improve ROM and RAM allocation control through [config-memmap.h](../include/configs/config-memmap.h) and [config-memmap.s](../include/link/config-memmap.s).
- Maintain a fixed pointer list through [Reloc.event](../Reloc/Reloc.event) so C hacks and FEBuilder patches can share data locations.
- Expand battle-system behavior, including a larger battle-hit budget of 20.

## Support Data Note

Support progress is currently stored in the BWL table, while the vanilla support table is reused for skill storage.

If a unit does not have BWL support data, meaning its character ID is greater than `0x45`, the kernel falls back to preloaded supports. In that case, the unit's support level may not grow during normal gameplay.
