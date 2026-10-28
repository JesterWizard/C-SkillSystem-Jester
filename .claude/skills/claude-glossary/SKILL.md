---
name: claude-glossary
description: Build a verified "where to look and how to template it" glossary for a decomp or ROM-hack repo (a lean root CLAUDE.md plus one doc per kind of change, e.g. skills, features, functions), so later sessions don't rediscover the process. Use when the user asks for a Claude glossary, a CLAUDE.md, or per-type how-to docs for a repo like this.
---

# Build a Claude glossary for a decomp / ROM-hack repo

The result is a lean root `CLAUDE.md` plus one glossary directory per **kind of change** the
repo gets (for example `Documentation/SkillTypes/` and `Documentation/FeatureTypes/`). Each
directory holds a `README.md` index, a `Common.md` with the steps every change of that kind
needs, and one doc per type. Every path and symbol in those docs is machine-checked, and the
claims behind them are spot-checked against the source.

`check_doc.py` sits next to this file. Run it from the target repo's root:
`python3 <this dir>/check_doc.py --exclude-dir <glossary dir> <docs...>`

## 1. Recon (you, read-only)

1. Snapshot the tree: `git status --porcelain > <scratchpad>/baseline.txt`. Glossary work
   must only add docs, so every later diff is compared with this.
2. Find out how the repo builds and checks itself (README, makefile targets, setup docs).
   Matching decomps usually have `make compare`; ROM-hack buildfiles usually just build a
   ROM. Write down the one command that proves the build still works.
3. Find the **kinds of change** the repo gets. Good signals:
   - `git log --oneline -60`: what recent commits add (skills, features, items, matched
     functions, mods).
   - Folder taxonomy (e.g. `SkillEffects/<Category>/`, `src/<subsystem>/`).
   - Per-category data or text files (e.g. `Skills_<Category>.txt`). These often *are*
     the type list.
   - Engine hook lists, dispatch tables, and `gp*` pointer tables.
4. For each kind, **trace one real example end to end**:
   - `git show --stat <commit that added it>` gives the full touch list.
   - `git grep -n -w <its symbol>` shows every place that names it.
   - Do a small one and a complex one (e.g. a passive skill and a menu skill) so you can
     tell universal steps from type-specific ones.
5. Read the existing human docs, but treat them as suspect. In the first repo this was
   built for, the main skill doc had wrong paths and broken links. Note stale docs as
   findings and don't copy paths from them.

## 2. Design (you decide; workers never do)

- One glossary dir per kind of change, with a `README.md` index, a `Common.md`, and one
  doc per type. Group small types that share a mechanism into one doc, so each doc stays
  a size one worker can research.
- Fix the per-type doc structure in advance, so every doc reads the same:

  ```
  # <Type>
  > Text/data file · Code dir · Hook / entry point
  ## What it is            (1-3 sentences)
  ## How the engine calls it   (chain from engine to code, path:line)
  ## Where to look         (table: file | role)
  ## Add one: steps        (step 1 = "Do the common steps in Common.md")
  ## Template              (adapted from a named real example, placeholder name Example)
  ## Worked examples       (2-4 real ones, with paths)
  ## Gotchas               (evidence only; anything else marked "unverified")
  ```

- The README has these sections:
  - "Pick a type": a table with columns "If it... | doc | entry point".
  - A table mapping every per-category data or text file to its doc, so any existing
    item can be located.
  - A grep recipe for finding an existing item.
  - A glossary of terms, with `path:line` for each.
  - "Known stale docs and hazards".
- Use one placeholder name everywhere (`Example`, `SID_Example`). The checker skips it.

## 3. Brief and dispatch

1. Write a **shared preamble** file in the scratchpad. Every worker reads it first. It
   contains:
   - Project context, in 3-4 lines.
   - **Overrides** for anything in the worker's standing rules that doesn't apply to this
     repo. Copied agent definitions often mention another project's tools.
   - The rules: docs only, no build, no state-changing git, edit only the named files.
   - The **facts you already verified**: the common touch list, hook lists, testers, and
     known pitfalls, each with a path.
   - The required doc structure.
   - Accuracy rules: `ls` every path, grep every symbol, give `path:line` for key claims,
     say "unverified" instead of guessing, and run the checker until it prints
     `RESULT: OK`.
   - A line saying: "If something here is contradicted by the code, the code wins; report
     it."
2. Write one brief per worker. Each brief names its exact output files, the text or data
   files and code dirs for its types, 2-3 example items to trace, and the specific
   questions to answer.
3. Dispatch the type-doc workers **in parallel**. Docs-only workers writing different new
   files don't conflict. Use cheap workers (e.g. Haiku); the checker and your review are
   the quality gate.
4. Run the **README/index worker last**, once every type doc is approved, because it
   summarises them. That worker also adds the short pointer to `CLAUDE.md`.

## 4. Verify every check-in (never skip)

A worker's report is a claim, not proof. For each one:

1. Run the checker yourself on its files.
2. Check `git status --porcelain` against the baseline. Only the named files may differ.
3. Spot-check **at least three claims**:
   - the template against the real code it says it was adapted from (open both);
   - one "how the engine calls it" link;
   - one item from its "please verify" or "unsure" list.
4. **Compile-check any numeric claim** the worker computed by hand, such as enum values or
   free IDs. A `_Static_assert` file built with the project's compiler and
   `-fsyntax-only` is enough.
5. Then decide:
   - **Approve.**
   - **Revise:** send numbered exact fixes to the same worker, so it keeps its context.
     Allow at most 2 rounds.
   - **Reject:** revert only that worker's files.

Errors these checks have caught before:

- A doc claiming list entries after a `true` return "never run". The walker advanced the
  index before calling, so they still run, just on a later frame.
- A list label cited under the name of the *pointer* to it. The label and the pointer live
  in different files.
- A link to `Common.md` removed just to make the checker pass while `Common.md` didn't
  exist yet. Allow that one failure instead.
- An ordered list described as being "in this order" when it wasn't.
- Facts in **your own preamble** that were wrong, such as which enum files the build reads
  and how IDs are assigned. Accept the correction, then grep the other docs to make sure
  none of them repeated the wrong fact.

## 5. CLAUDE.md: keep it lean

`CLAUDE.md` is loaded into every session, so it should hold only:

- what the repo is (1 line);
- the build command and where the config lives;
- one pointer per glossary ("Adding a skill: read `<dir>/README.md`, then `Common.md`,
  then the type doc");
- the contributing rules;
- **pitfalls** that cost real time: broken compiler or runtime helpers, nearly full
  sections, generated files you must not edit, stale copies to exclude from grep,
  committed build output with hand edits.

Leave out directory tours, target lists, and line numbers (they go stale); the glossary
holds those. Aim for about 30-40 lines.

## 6. Finish

1. Run the checker over every glossary doc and `CLAUDE.md`.
2. Confirm the diff against the baseline shows docs only.
3. Don't commit unless asked.
4. Report to the user:
   - a task table showing approved, revised and rejected work;
   - **what review caught**, which tells the user how far to trust the worker output;
   - **bugs found in the code** while researching. Report them; don't fix them inside a
     docs task. Building the first glossary turned up an array overrun, a skill using
     another skill's value, and a `make clean` recipe that deleted 632 tracked files;
   - stale human docs and hazards, which go in the README's hazards section.

## Adapting to the repo

| Repo shape | Kinds of change | Build check | Typical extra docs |
|---|---|---|---|
| ROM-hack buildfile (C + EA/lyn, like this one) | skills, features, items, events | `make` | hook lists, RAM/save layout, designer config, graphics pipeline |
| Matching decomp (`make compare`) | matching a function, naming symbols, documenting a struct, adding a mod | `make compare` OK | symbol/naming workflow, asm-to-C patterns, non-matching tricks |
| Disassembly / asm-first | labelling, splitting data, adding a hack | rebuilt ROM checksum | label conventions, data-format tables |

For matching decomps, add a Gotchas convention: any doc claim about generated code must be
backed by a `make compare` run or a disassembly line, not by reading C alone.
