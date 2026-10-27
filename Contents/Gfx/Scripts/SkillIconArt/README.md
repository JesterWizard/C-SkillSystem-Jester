# Skill icon art

Hand-drawn icons for 348 skills that used to be on the WIP icon, plus the 24
"...Plus" icons built from them (or from an existing base icon). Art by
Claude; see `Documentation/Features/Credits.md`.

Files:

- `glyphs.txt` - the drawings. One 12x12 glyph per skill, with its
  background colour. The format is described at the top of `render.py`.
- `render.py` - turns `glyphs.txt` into 16x16 icons inside the standard skill
  icon border (`border.txt`, the border most hand-made icons share), adds the
  outline, and checks every icon against the others and against the icons in
  `Contents/Gfx/Sources/SkillIcon/` (exact copies, mirror images, and
  near-copies that differ in fewer than 20 pixels).
- `plus.py`, `plus.txt`, `plus_template.txt` - the "...Plus" icons: the base
  skill's icon with the gold border and red plus badge used by the hand-made
  Plus icons (`plus_template.txt` is what all of those share; the rest comes
  from `CantoPlus.png`).

Both scripts need Python 3 with Pillow.

## Changing an icon

Edit its entry in `glyphs.txt`, then from the repo root:

    python3 Contents/Gfx/Scripts/SkillIconArt/render.py /tmp/icons /tmp/icons_sheet.png Contents/Gfx/Scripts/SkillIconArt/glyphs.txt

It must report 0 warnings. Look at the sheet, then copy the PNG you changed
from `/tmp/icons` into `Contents/Gfx/Sources/SkillIcon/` and rebuild. If the
skill has a Plus version, remake that too:

    python3 Contents/Gfx/Scripts/SkillIconArt/plus.py /tmp/plus Contents/Gfx/Scripts/SkillIconArt/plus.txt /tmp/icons Contents/Gfx/Sources/SkillIcon

The scripts write to a separate folder on purpose: the icons they make live
in the folder they compare against.

To use an icon drawn some other way, overwrite
`Contents/Gfx/Sources/SkillIcon/<Name>.png` (16x16, the shared skill icon
palette) and delete its entry from `glyphs.txt`.
