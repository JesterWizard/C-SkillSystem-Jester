---
name: compile-undefined-globals
description: "Use when the compiler reports undefined identifiers such as _L17_4 or other unexpected global references. Decide whether the value should be const or whether it needs a memmap reservation in include/link/config-memmap.s plus an extern declaration, based on whether the value must change at runtime."
---

# Compile Undefined Globals

Use this skill when a build fails because a C file references an identifier that does not resolve cleanly at compile or link time.

## Use This Skill When

- The compiler reports an undefined identifier such as `_L17_4`.
- A C file references a global value that is not declared or not linked correctly.
- You need to decide whether a value should be `const` or placed in `include/link/config-memmap.s`.
- A new global is intended to live in RAM but has no matching `extern` declaration.

## Rule

- If the value never needs to change, make it `const`.
- If the value must be mutable at runtime, reserve it in `include/link/config-memmap.s` and add the matching `extern` declaration.
- Do not leave a global half-defined: the declaration, definition, and storage model must agree.

## Decision Flow

1. Identify the symbol that triggered the error.
2. Determine whether the value is read-only data or runtime state.
3. If it is read-only, convert the definition to `const` and remove any unnecessary RAM reservation.
4. If it is runtime state, add or verify the `_kernel_malloc` reservation in `include/link/config-memmap.s`.
5. Add or verify the corresponding `extern` declaration in the relevant header.
6. Rebuild to confirm the undefined identifier is gone.

## Review Checklist

- The symbol name matches the intended global.
- Read-only data is marked `const`.
- Mutable data has a memmap reservation and an `extern` declaration.
- No stale declaration points at a removed or renamed symbol.
- The build no longer emits the undefined identifier.

## Common Failure Modes

- A file uses a global as if it already exists, but the definition was never added.
- A value that should be read-only was left non-`const`, which makes the linker expect storage.
- A mutable global was added in code but never reserved in the memmap.
- The declaration exists, but the symbol name differs from the actual definition.
