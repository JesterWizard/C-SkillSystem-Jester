---
name: demo-gif
description: Make a captioned, step-by-step demo GIF of a feature, skill or commission from the emulator (gbarun), in the house style - a caption bar under every frame that explains what is happening, and a box outlining the part of the screen the feature changes. Use whenever the user asks for a GIF, a demo or a recording of something in the game.
---

# Demo GIF

A demo GIF must show the feature to a new user **with no ambiguity**: someone who has never heard of it should
understand, from the GIF alone, what the feature does and what rule it enforces. A plain recording is not enough.

Every GIF made with this skill has:

- The game at 2x (480x320), and under it a 70 px dark caption bar: the feature's name in gold capitals, then one or
  two lines of white caption saying what is happening **right now** and why.
- A gold box outlining the screen element the feature is about (an inventory row, a stat, a menu entry), whenever
  that screen is up.
- Several short scenes, each making one point, in this order:
  1. **What it is**: a still screen with the element outlined, and a caption naming it ("Slot 5 (outlined) holds
     one accessory...").
  2. **The normal case** working.
  3. **Each rule or limit** being enforced, set up so the limit is the only possible explanation (e.g. a full
     inventory of weapons, so the refusal can only be because of the new rule). Show the player hitting the limit:
     grayed rows, a refusal, the error message.
  4. **The end state** (the outlined element after the change).

## Plan before recording

1. Read the feature's doc or README: list what it does and every rule it enforces.
2. Write the scenes and their captions first, in a short table, and show it to yourself before recording:
   - Captions are statements, not labels: "...but her inventory is full: slot 5 only takes accessories."
   - Use items, units and classes whose names make the point. A ring or shield reads as an accessory; a Vulnerary
     doesn't. Avoid anything that could be mistaken for vanilla behaviour.
   - Name the ROM's names for things (check the in-game text, e.g. Delphi Shield shows as "Fili Shield").
3. Each scene starts from the same save state, with its setup written to RAM (`unit_items`, `raw`), so scenes
   don't depend on each other.

## Tools

`scene.py` and `compose.py` are next to this file. Work in a scratchpad folder (`WORK`), not the repo.

1. **Start state.** Boot the ROM under test with `Tools/cloud/boot-to-prologue.txt` (or `boot-to-prep.txt`), go
   to the situation the demo needs and `save ready.ss` in `WORK`. Save states belong to one ROM build: make them
   again after a rebuild. Find unit and RAM addresses from the ROM's `.sym` (or the decomp for a clean ROM),
   don't guess.
2. **Record each scene** with `Scene` (see `scene.py`'s docstring):

   ```python
   import sys; sys.path.insert(0, '/home/user/C-SkillSystem-Jester/.claude/skills/demo-gif')
   from scene import Scene, unit_items
   s = Scene(WORK, 's2', setup=unit_items(EIRIKA, [RAPIER, IRON_SWORD]) + enemy_setup)
   s.caption('Eirika has 4 weapons. Slot 5 is empty.|The soldier drops an Iron Sword...'); s.rec(40)
   s.key('A', 4, 200)
   s.caption('...but her inventory is full:|slot 5 only takes accessories.'); s.rec(220)
   s.save()
   ```

   then `cd WORK && gbarun ROM s2.txt`. `Scene` turns battle animations off (`ANIMS_OFF`) to keep scenes short;
   pass `prelude=()` when the animation is the feature.
3. **Compose**: write a `config.json` (format in `compose.py`'s docstring: title, scenes, stride, duration, boxes
   with GBA-pixel rects) and run `python3 compose.py WORK WORK/config.json`. A box with `only_if` is drawn only
   while the frame matches a reference frame's region, so it vanishes when its screen closes.
4. **Check `WORK/contact.png`** (30 frames across the GIF) and, for any doubtful step, a denser sheet of that
   scene's frames. Check every caption matches what is on screen at that moment, the box sits on the right
   element, and every menu choice landed on the intended row. Fix and re-record until it does.

## Input timing (gbarun)

- Hold menu keys 4 frames (`key('A', 4, ...)`), and leave 30-40 frames between presses: shorter presses drop.
- Wait until a menu is actually open before moving its cursor (record ~200 frames after an item popup): presses made
  while it opens are lost and the cursor lands on the wrong row. FE menus don't wrap, so UP from the top does nothing.
- Yes/No prompts need ~150 frames before LEFT; item popups ~420 frames before R opens the stat screen.
- Give each caption at least ~90 recorded frames (about 2 s in the GIF) so it can be read; use `wait` (not
  recorded) to skip dead time.

## Size and delivery

- Default `stride` 2 and `duration` 40 ms play at real speed. Aim for under ~5 MB: shorten waits, cut scenes or
  raise the stride before lowering quality.
- Put the GIF next to the feature's doc (`Documentation/Gifs/<Name>.gif` for kernel features, the commission's
  folder for commissions), show it at the top of the doc with
  `<p align="center"><img src="..." alt="..." width="480"/></p>`, and write an italic caption under it describing
  the demo's setup and steps.
- Send it to the user with SendUserFile (`display: render`). Don't commit unless asked.
