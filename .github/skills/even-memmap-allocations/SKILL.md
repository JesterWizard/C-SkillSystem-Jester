---
name: even-memmap-allocations
description: "Use when editing include/link/config-memmap.s or any RAM allocation table to ensure every _kernel_malloc reservation stays even-sized. Prefer 2-byte-or-larger allocations, and if a symbol needs 1 byte, add immediate padding or pack it with another byte-sized allocation so the total remains even."
---

# Even Memmap Allocations

Enforce even-sized memory reservations in `include/link/config-memmap.s`.

## Use This Skill When

- The user adds or changes `_kernel_malloc` entries in `include/link/config-memmap.s`.
- The user asks to reserve RAM for a new symbol or state block.
- The user wants to prevent odd-byte allocations from shifting later addresses off parity.

## Rule

- Keep every individual reservation even-sized whenever possible.
- If a feature naturally needs 1 byte, do not leave it as a standalone odd allocation.
- Instead, add one byte of padding immediately after it, or combine it with another byte-sized field so the total reservation is even.
- Re-check the next allocation after any change; the running address must still remain aligned to an even byte boundary.

## Review Checklist

1. Inspect the new or changed `_kernel_malloc` line.
2. Confirm the requested size is even.
3. If the size is odd, add an immediate padding byte or pair it with another odd-sized field.
4. Verify the following reservation still starts on an even address.
5. Rebuild or reparse the memmap to confirm no downstream symbol drift.

## Preferred Patterns

- Use 2, 4, 6, 8, ... byte reservations.
- Group related byte flags into a single even-sized block.
- Reserve one extra padding byte next to any unavoidable 1-byte field.

## Avoid

- Standalone 1-byte allocations.
- Leaving a trailing odd-sized block that forces later symbols onto an odd address.
- Fixing parity downstream instead of correcting the current reservation.