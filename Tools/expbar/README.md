# expbar

Draws the EXP bar styles that the Options screen's "EXP Bar" row picks from. See
[`Documentation/Features/ExpBarStyles.md`](../../Documentation/Features/ExpBarStyles.md).

- `styles.py`: the styles. Each is a dict of its palette, window, label, bar (full and empty drawings of the left cap,
  a body tile and the right cap) and digits; the file's docstring lists every key.
- `expbar.py`: draws them into `Kernel/Wizardry/ExpBarStyles/Gfx/<Name>.png` (64x64, indexed, 16 colours). The build
  makes each PNG's `.4bpp` and `.gbapal`.

```bash
python3 Tools/expbar/expbar.py
```

```bash
python3 Tools/expbar/expbar.py --only Blade --preview /tmp/expbar.png
```

`--preview` writes each style's map window at 99 and 29 EXP, from its tiles, with no emulator (the bar's fill there is
approximate). For a style in game, build and run `python3 Tools/cloud/smoketest.py --only expbar`, or set the option on
the Options screen.

A new style also needs its labels in `Kernel/Wizardry/ExpBarStyles/ExpBarStyles_Installer.event` and a row in
`Kernel/Wizardry/ExpBarStyles/Source/ExpBarStyles.c` (the feature doc's "Adding a style").
