[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://makeapullrequest.com)

[Discussion on PR review](https://github.com/MokhaLeee/fe8u-cskillsys-kernel/issues/249)

# Contributing

This project is intended to be a community-maintained C-SkillSys buildfile, and pull requests are welcome. Reviews are intentionally strict because this repository touches core engine behavior and regressions can spread widely.

## Mandatory Requirements

1. Skill code must compile safely when the skill ID is absent.

For pull requests that add new skills, the kernel must still compile and run even if the skill ID is commented out. Guard skill-specific code with the following pattern. See [SkillSys.md](./SkillSys.md) section 3.1 for background.

```c
#if defined(SID_xxx) && (COMMON_SKILL_VALID(SID_xxx))
    // Some effects
#endif
```

1. `lynjump` replacements must use defensive checks.

To reduce breakage from future C-Lib changes, rewritten functions that rely on `lynjump` should be guarded explicitly.

Add `LYN_REPLACE_CHECK` before the rewritten function:

```c
LYN_REPLACE_CHECK(ComputeBattleUnitAttack);
void ComputeBattleUnitAttack(struct BattleUnit *attacker, struct BattleUnit *defender)
{
    // ...
}
```

Also create a `LynJump.event` file in the same directory and keep the matching lynjump data there:

```event
PUSH
ORG $2aabc
ALIGN 4
WORD $46C04778 $E59FC000 $E12FFF1C
POIN ComputeBattleUnitAttack
POP
```

## Code Style

C-SkillSys follows the [Linux kernel coding style](https://www.kernel.org/doc/html/v4.10/process/coding-style.html). Check your C files with:

```bash
./check_codingstyle.sh <path-to-your-c-file>
```

Please resolve all reported ERRORs. Also resolve all WARNINGs unless you have a clear reason not to.

## Suggestions

1. Spend time studying [FE8 decomp](https://github.com/FireEmblemUniverse/fireemblem8u), not just writing new code.

2. Prefer more C and less ASM.

ASM is still necessary in some places, but C should be the default when the same behavior can be implemented cleanly there. PRs that introduce large amounts of ASM without a strong reason are unlikely to be merged.

1. Watch performance-sensitive code paths carefully.

Some areas have an outsized effect on game feel and runtime cost:

- Battle calculation functions
- Status getter functions
- `SkillTester`
- Map-task logic in `gProc_MapTask`
- Movement generation code such as `GenerateUnitCompleteAttackRange`
