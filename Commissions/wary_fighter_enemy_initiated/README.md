# Wary Fighter (Enemy-Initiated)

Same as normal Wary Fighter, but only when an enemy starts the fight. Player and NPC initiated combat is vanilla.

Edit `WaryFighterUnits.event` (`$FF` = everyone, `$00` = end). Table edits do not need a rebuild.

```
WaryFighterUnit($01) // Eirika
WaryFighterUnit($00)
```

Insert EA on `Installer.event`. Hook is `BattleGetFollowUpOrder` at `$2AF90`. Body at `$1000000`.

Conflicts: Skill System, [wary_fighter](../wary_fighter/), anything else on `$2AF90` or `$1000000`.

Run `make` only if you change the `.c` file.
