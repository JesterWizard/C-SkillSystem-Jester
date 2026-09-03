# Standalone FEBuilder Patches

Self-contained Event Assembler patches extracted from the integrated C Skill System. Each feature lives in its own folder and installs on clean FE8U without the kernel.

| Patch | Description |
|-------|-------------|
| [two_random_number_growths](two_random_number_growths/) | Uses 2RN for fractional level-up growth rolls |
| [guaranteed_lvup](guaranteed_lvup/) | Retries empty level-ups up to 10 times with +10% growth |
| [custom_fog_sight](custom_fog_sight/) | Per-class fog vision bonuses |

## Adding a new patch

Use the project skill **extract-standalone-feature** (`.agents/skills/extract-standalone-feature/SKILL.md`).

Reference implementation: `two_random_number_growths/`.
