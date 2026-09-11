# Wary Fighter

Stops doubling for both fighters. No HP gate. Brave weapons still strike twice.

Edit `WaryFighterUnits.event` only. Delete `//` to enable a line. Keep both `$00, $00` end-of-table lines.

- `WARY_ALWAYS` — no doubling in any combat
- `WARY_ENEMY_INIT` — no doubling only when an enemy initiates

**Everyone:** change `BYTE WARY_LIST` to `BYTE WARY_ALWAYS` or `BYTE WARY_ENEMY_INIT`.

**Specific units/classes:** leave `WARY_LIST`, then uncomment rows. IDs are hex from FEBuilder’s Character / Class editors. `$FF` = everyone else. Priority: unit ID > class ID > unit `$FF` > class `$FF`. Always-on beats enemy-init if both fighters differ.

Eirika always-on, Seth only when an enemy initiates, every General always-on:

```
WaryFighterUnits:
WaryFighterUnit($01, WARY_ALWAYS)     // Eirika
WaryFighterUnit($02, WARY_ENEMY_INIT) // Seth
WaryFighterUnit($00, $00)             // end of table

WaryFighterClasses:
WaryFighterClass($0B, WARY_ALWAYS)    // General
WaryFighterClass($0C, WARY_ALWAYS)    // General (F)
WaryFighterClass($00, $00)            // end of table
```

Leave unused rows commented (`//`) or delete them. Do not comment out the `$00, $00` lines.

## Install (FEBuilder)

Edit the lists first, then **Advanced Editors → Insert EA →** `Installer.event` → **Load Script**. No compile needed.

To change the lists later, insert again in the same free space on your existing ROM.

Hook `$2AF90`, free space `$1000000`. Conflicts: Skill System. `make` only if you change the `.c`.
