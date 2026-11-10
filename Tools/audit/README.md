# Unreachable content audit

`unreachable.py` lists content that is built into the ROM but that nothing in the game reaches: event entry points
(`*ASMC`) no event calls, messages no source names, and items nothing gives or sells. It came out of finding that Snek
was a whole minigame, with texts, records and two guild quests, that no event started.

```bash
python3 Tools/audit/unreachable.py          # leads to check (exit 1 if there are any)
python3 Tools/audit/unreachable.py --all    # also the known ones, with the reason each is known
```

It searches the sources with comments removed, so a commented-out call does not count. A name can still be reached in
ways a text search cannot see (a computed message id, a table built by a script), so a hit is a lead, not a verdict.
Known groups are in `KNOWN` and `KNOWN_PATTERNS` with their reasons. Run it after adding a feature: a new entry point,
message or item that nothing reaches shows up.

## Findings (first run)

Fixed:

- **Snek** could not be started: the Pied Piper in Port Kiris now runs it (PR 663).
- **Caer Pelyn and Jehanna Hall** could never open: their story chapters were past the end of the story table (PR 674).
- **Rare Candy** (+100 EXP) existed but nothing gave it: it is a guild board prize, 100 tokens.
- **Tonic names** were English strings in `DuraItem.c` while their messages (`MSG_ITEM_TONIC_*_NAME`) went unused: the
  names now come from the messages.

Reported, for the campaign's author:

- **Chapter 10 is a stub.** `CustomCampaign/Chapters/10/events/events.c` has its event group commented out and its
  texts are one-line placeholders, so the custom campaign ends at Chapter 9.
- **No ending yet.** The 172 paired endings and the credits roll (`StartCreditsProc_ASMC`) wait for the campaign's end.
- **Placeholders.** Most Heroes card biography pages and the later description stages of bosses are empty; the written
  biographies are the five in `Data/MapMenus/CharacterBiographies.c`.

Left as leads to review (49 messages that no code names, mostly UI strings of features that now use other text or are
not installed):

`FMU_exitMenuCommandText`, `FMU_exitMenuHelpText`, `MSG_AVO`, `MSG_BEXP_APPLIED_DESC`, `MSG_CROSSING_K_PAUSE`, `MSG_CRT`, `MSG_DOG`, `MSG_ITEM_RemoveSkillScrollDesc`, `MSG_ITEM_RemoveSkillScrollName`, `MSG_MENU_OPTION_GAMEOVER_QUOTES_DESC`, `MSG_MENU_OPTION_GAMEOVER_QUOTES_TITLE`, `MSG_MSS_ARTS`, `MSG_MSS_LaguzBarDesc`, `MSG_MSS_SkillCapacity`, `MSG_MSS_SkillCapacityDesc`, `MSG_MSS_SkillPoints`, `MSG_MSS_SkillPointsDesc`, `MSG_MenuCommand_Allocate_NAME`, `MSG_PREP_SCREEN_DESC_CANCEL`, `MSG_PREP_SCREEN_TITLE_CANCEL`, `MSG_PopupSkillLearned`, `MSG_SELECT_WEAPON`, `MSG_SILENCER`, `MSG_SKILL_Arise_FRtext`, `MSG_SKILL_Blacksmith_FRtext`, `MSG_SKILL_Capture_FRtext`, `MSG_SKILL_DivineReprieve_Confirm`, `MSG_SKILL_HealingFocus_FRtext`, `MSG_SKILL_LightRune_FRtext`, `MSG_SKILL_Mine_FRtext`, `MSG_SKILL_Rally_FRtext`, `MSG_SKILL_StealPlus_FRtext`, `MSG_STATUS_SLEEP_NAME`, `MSG_SkillEffect_Lunge_Confirm`, `MSG_SkillRemoved`, `MSG_SkillSwapSkillChoice`, `MSG_WM_MANAGE_SKILLS_CHOOSE_UNIT`, `MSG_WM_MANAGE_SKILLS_OPTION_EDIT`, `MSG_WM_MANAGE_SKILLS_OPTION_REMOVE_ALL`, `MSG_WM_SKILL_SHOP_IDLE_DIALOGUE`, `R_TEXT_COMMAND_ESCAPE`, `R_TEXT_COMMAND_REFUGE`, `SecretShopOutOfStockText`, `TARGET_TEXT_REFUGE`, `TILE_COMMAND_TEXT_ESCAPE`, `TILE_COMMAND_TEXT_REFUGE`, `VendorOutOfStockText`, `WLC_menuCommandText`, `WLC_menuHelpText`
