# Copilot Workspace Instructions

- When a user asks to create or rewrite documentation, use the skill at `.github/skills/documentation-style-docs/SKILL.md`.
- Default to the architecture and section order defined by that skill unless the user requests a different structure.
- Keep docs contributor-oriented: player impact first, then implementation details and concrete code locations.
- When editing `include/link/config-memmap.s` or any RAM allocation table, use the skill at `.github/skills/even-memmap-allocations/SKILL.md` to keep `_kernel_malloc` reservations even-sized and padded if needed.
