# Wary Fighter

Listed units stop follow-up attacks for both sides. No HP gate. Brave weapons still strike twice.

Edit `WaryFighterUnits.event` (`$FF` = everyone, `$00` = end). Table edits do not need a rebuild.

```
WaryFighterUnit($01) // Eirika
WaryFighterUnit($00)
```

Insert EA on `Installer.event`. Hook is `BattleGetFollowUpOrder` at `$2AF90`. Body at `$1000000`.

Conflicts: Skill System, the [enemy-initiated variant](../wary_fighter_enemy_initiated/), anything else on `$2AF90` or `$1000000`.

Run `make` only if you change the `.c` file.
