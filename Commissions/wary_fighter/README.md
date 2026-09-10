# Wary Fighter

Listed units stop follow-up attacks for both sides. No HP gate. Brave weapons still strike twice.

Edit `WaryFighterUnits.event`:

- `WaryFighterGlobalMode`: `0` = unit/class lists, `1` = everyone always-on, `2` = everyone enemy-init only
- Lists (used when mode is `0`): `WaryFighterUnit(pid, version)` and `WaryFighterClass(jid, version)` — `1` always-on, `2` enemy-init. Specific unit IDs beat specific class IDs. `$FF` is the fallback (unit `$FF` beats class `$FF`). `$00, $00` ends each table.

```
WaryFighterGlobalMode:
BYTE WARY_LIST

WaryFighterUnit($01, WARY_ALWAYS)
WaryFighterUnit($00, $00)

WaryFighterClass($0B, WARY_ALWAYS)
WaryFighterClass($FF, WARY_ENEMY_INIT)
WaryFighterClass($00, $00)
```

Insert EA on `Installer.event`. Hook `$2AF90`, body `$1000000`. Conflicts: Skill System.

`make` only if you change the `.c`.
