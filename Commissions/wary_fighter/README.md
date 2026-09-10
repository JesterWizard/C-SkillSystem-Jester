# Wary Fighter

Listed units stop follow-up attacks for both sides. No HP gate. Brave weapons still strike twice.

Edit `WaryFighterUnits.event`:

- `WaryFighterGlobalMode`: `0` = unit list, `1` = everyone always-on, `2` = everyone enemy-init only
- List (used when mode is `0`): `WaryFighterUnit(pid, version)` — `1` always-on, `2` enemy-init. `$FF` is the fallback. `$00, $00` ends the table.

```
WaryFighterGlobalMode:
BYTE WARY_LIST

WaryFighterUnit($01, WARY_ALWAYS)
WaryFighterUnit($FF, WARY_ENEMY_INIT)
WaryFighterUnit($00, $00)
```

Insert EA on `Installer.event`. Hook `$2AF90`, body `$1000000`. Conflicts: Skill System.

`make` only if you change the `.c`.
