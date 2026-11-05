# Hex (CONFIG_HEX_CARDS)

Hex card game port. The switch is `CONFIG_HEX_CARDS` in `include/configs/configs.h`.

- `Source/HexCore.c`, `Source/HexCards.c`: rules engine and card collection.
- `Source/HexPrep.c`: entry point called from the prep menu (`AtMenu.c`, the "Hex" row).
- `Source/HexScreen.c`: placeholder full-screen proc (`ProcScr_HexScreen`).
- `Hex_Installer.event`: included from `Kernel/Wizardry/wizardry.event` under `CONFIG_HEX_CARDS`.

## Toggling CONFIG_HEX_CARDS in a cloud session

`Tools/cloud/build.sh` does not regenerate tracked `.lyn.event` files whose `.c` is unchanged in git. So after turning the switch on or off, the `.lyn.event` outputs of the guarded `.c` files keep their old content and the build fails (for example `DuraItem.lyn.event:33:6 Undefined identifier: HexCard_NameId` when the switch is off).

Delete the guarded outputs, then build:

```
rm -f Data/ItemSys/Source/Items.lyn.event \
  Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/DuraItem.lyn.event \
  Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ItemInfos.lyn.event \
  Kernel/Wizardry/SkillEffects/MenuSkills/Reinforce.lyn.event \
  Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.lyn.event \
  Data/Hex/HexCardTypes.lyn.event
Tools/cloud/build.sh
```

`build.sh` regenerates them because a missing file is not protected. Afterwards, `git diff` on those files should show only the expected changes for this feature (AtMenu.lyn.event). Do not change `build.sh` for this.
