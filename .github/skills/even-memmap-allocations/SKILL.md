---
name: even-memmap-allocations
description: "Use when editing include/link/config-memmap.s or any RAM allocation table to ensure every _kernel_malloc reservation stays even-sized. Run make ramcheck before adding new RAM allocations and savecheck before adding new suspend RAM allocations, then decide whether the space is sufficient. Prefer 2-byte-or-larger allocations, and if a symbol needs 1 byte, add immediate padding or pack it with another byte-sized allocation so the total remains even."
---

# Even Memmap Allocations

Enforce even-sized memory reservations in `include/link/config-memmap.s`.

## Use This Skill When

- The user adds or changes `_kernel_malloc` entries in `include/link/config-memmap.s`.
- The user asks to reserve RAM for a new symbol or state block.
- The user wants to prevent odd-byte allocations from shifting later addresses off parity.
- The user adds a new global variable that is not `const` and needs it reserved in the memmap.
- The user is about to add new RAM allocations or suspend RAM allocations and needs to verify available space first.

## Rule

- Before adding a new RAM allocation, run `make ramcheck` to measure remaining RAM space.
- Before adding a new suspend RAM allocation, run `make savecheck` to measure remaining suspend RAM space.
- If the remaining space is not enough, stop and report the outcome in the chat window so the operator can decide the next steps.
- Every new non-`const` global variable must be reserved in `include/link/config-memmap.s` with a matching `_kernel_malloc` entry.
- Keep every individual reservation even-sized whenever possible.
- If a feature naturally needs 1 byte, do not leave it as a standalone odd allocation.
- Instead, add one byte of padding immediately after it, or combine it with another byte-sized field so the total reservation is even.
- Re-check the next allocation after any change; the running address must still remain aligned to an even byte boundary.

## Review Checklist

1. Inspect the new or changed `_kernel_malloc` line.
2. Run `make ramcheck` for ordinary RAM changes and `savecheck` for suspend RAM changes.
3. Confirm the remaining space is sufficient before committing to the allocation.
4. Confirm every new non-`const` global has a reservation in the memmap.
5. Confirm the requested size is even.
6. If the size is odd, add an immediate padding byte or pair it with another odd-sized field.
7. Verify the following reservation still starts on an even address.
8. Rebuild or reparse the memmap to confirm no downstream symbol drift.

## Preferred Patterns

- Use 2, 4, 6, 8, ... byte reservations.
- Group related byte flags into a single even-sized block.
- Reserve one extra padding byte next to any unavoidable 1-byte field.
- If capacity is insufficient, report the measured remainder in chat and wait for an operator decision instead of forcing a layout change.

## Avoid

- Standalone 1-byte allocations.
- Leaving a trailing odd-sized block that forces later symbols onto an odd address.
- Fixing parity downstream instead of correcting the current reservation.