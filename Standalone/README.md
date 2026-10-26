# Standalone FEBuilder Patches

Self-contained Event Assembler patches extracted from the integrated C Skill System. Each feature lives in its own folder and installs on clean FE8U without the kernel, this repo's Event Assembler includes, or Png2Dmp. Copy one folder and Insert EA on `Installer.event`.

| Patch | Description |
|-------|-------------|
| [two_random_number_growths](two_random_number_growths/) | Uses 2RN for fractional level-up growth rolls |
| [guaranteed_lvup](guaranteed_lvup/) | Retries empty level-ups up to 10 times with +10% growth |
| [restore_hp_on_level_up](restore_hp_on_level_up/) | Refills the player unit's HP when anyone in that battle levels up |
| [biorhythm_mechanic](biorhythm_mechanic/) | Per-character hit and avoid cycle that advances once per turn |
| [item_stack](item_stack/) | Unit-menu Stack command that merges duplicate items when total uses are at most 255 |
| [custom_fog_sight](custom_fog_sight/) | Per-class fog vision bonuses |
| [arena_show_opponent_in_advance](arena_show_opponent_in_advance/) | Shows arena opponent details before the wager prompt |
| [death_dance](death_dance/) | Rescued units can move when their rescuer dies |
| [promote_enemy_on_kill](promote_enemy_on_kill/) | Enemies auto-promote and gain stats when they score a kill |
| [promotion_on_max_level](promotion_on_max_level/) | Unpromoted units promote right after the battle where they reach level 20 (optionally trainees at level 10) |
| [custom_talk_icon](custom_talk_icon/) | Lex Talionus-style talk icon above the conversation partner |
| [auto_repair_weapons](auto_repair_weapons/) | Restores unbroken weapons to full durability at chapter transition |
| [infinite_durability](infinite_durability/) | Weapons never lose uses, and weapon durability numbers are hidden |
| [last_weapon_hit_crit](last_weapon_hit_crit/) | A strike that spends a weapon's last use is a guaranteed critical |
| [stat_page_promotions](stat_page_promotions/) | Fourth stat screen page listing a unit's promotion classes from Chapter 10 |
| [text_box_extension_layout](text_box_extension_layout/) | Help-box overflow: vanilla truncate, 5-line box, or 3-line pages |
| [custom_battle_quotes](custom_battle_quotes/) | Dual-character pre-battle quote matching with an editable table |
| [dynamic_weapon_slots](dynamic_weapon_slots/) | Per-class weapon-type-to-rank-slot mapping for custom types (knives, guns, etc.) |
| [alpha_blend_movement_sprites](alpha_blend_movement_sprites/) | Faded MU ghost at the pathfinding cursor tip (uses 50 bytes EWRAM) |
| [talk_on_level_up](talk_on_level_up/) | Character quotes after level-up stat gains (poor / good / great) |
| [goal_timer](goal_timer/) | Real-time chapter countdown goal; hitting zero is game over |
| [world_map_thought_bubbles](world_map_thought_bubbles/) | Chapter-specific thought bubbles on the world map node menu |
| [gameover_quotes](gameover_quotes/) | Random tip quotes on the game-over fog screen |
| [refuge](refuge/) | Unit menu command: take refuge in an adjacent ally (reverse Rescue) |
| [expanded_hp](expanded_hp/) | Unsigned HP cap 254 with 3-digit stat screen, minimug, and battle gauge |
| [text_engine](text_engine/) | Dialogue text engine: fonts, colours, box shapes, nameplates, letter and screen effects, popups, auto-advance, rainbow text, listener dimming and an optional SELECT conversation log (`TEXT_ENGINE_CHATLOG`) (uses 0x1A0 bytes EWRAM, 0x7A0 with the log) |
| [talk_conversation_exp_reward](talk_conversation_exp_reward/) | The unit that uses the Support command gains configurable EXP (default 10) after the on-map support conversation |
| [unlock_all_supports](unlock_all_supports/) | Extras > Support Conversation is always available, with every character and every C/B/A conversation unlocked |
| [konami_style_bonus_screen](konami_style_bonus_screen/) | Up Up Down Down Left Right Left Right B A on the title screen grants a random stat booster via Transfer Data (uses 2 bytes EWRAM) |
| [remove_move_path](remove_move_path/) | Hides the movement path arrow; units walk the shortest route to the chosen tile |
| [custom_droppable_item_icon](custom_droppable_item_icon/) | Blinking potion icon on map units that carry a droppable item |
| [stat_screen_allegiance_colors](stat_screen_allegiance_colors/) | Stat screen background tinted blue/red/green/gold by the unit's allegiance |
| [deny_stat_screen_access](deny_stat_screen_access/) | Configurable class list whose units can't be opened or browsed on the stat screen |
| [variable_unit_descriptions](variable_unit_descriptions/) | Per-chapter character description overrides on the stat screen via a text-ID table |
| [l_button_same_faction_cycling](l_button_same_faction_cycling/) | L button cycles through units of the same faction as the unit under the cursor |
| [max_simultaneous_support_conversations](max_simultaneous_support_conversations/) | Raises the per-unit support-rank cap (default 10), or optionally only the prep-screen counter |
| [no_wait_after_trading](no_wait_after_trading/) | Cancelling the action menu after trading returns the unit to movement instead of forcing Wait |
| [rescue_drop_move_again](rescue_drop_move_again/) | A dropped unit that hadn't acted yet can still move and act this turn |
| [fast_map_animations](fast_map_animations/) | All map sprites walk at top speed (one tile per frame) |
| [skip_intro](skip_intro/) | Skips the new-game opening monologue, the Prologue world-map narration and all chapter title cards |
| [levelup_modes](levelup_modes/) | Level-up stat-gain mode per difficulty (vanilla, separate-RNG random, fixed, 100%, 0%), with optional guaranteed level-ups (uses 8 bytes EWRAM) |
| [promote_trainees_in_chapter](promote_trainees_in_chapter/) | Trainees (Ross, Amelia, Ewan) promote right after the battle where they reach level 10 |
| [stat_gain_frame_speed](stat_gain_frame_speed/) | Frames the map level-up window waits between stat gains (vanilla 20, default 10) |
| [custom_defeat_quotes](custom_defeat_quotes/) | Defeat quotes that can depend on who landed the killing blow, from an editable table with vanilla fallback |
| [popup_item_icon_shine](popup_item_icon_shine/) | A white glint sweeps across item icons in "got item" popups |
| [chapter_names](chapter_names/) | FE7-style chapter titles drawn as text from each chapter's title text, plus the save menu chapter title fix |
| [res_terrain_window](res_terrain_window/) | The terrain window shows DEF, RES and AVO |
| [crit_cutin](crit_cutin/) | Critical hits by units with a portrait show a portrait cut-in over an electric band before the attack (battle animations) |
| [class_growths](class_growths/) | Adds or averages class growths into personal growths on level-up, with a promotion-screen growth preview (uses 128 bytes EWRAM) |
| [hit_decrease_on_range](hit_decrease_on_range/) | The attacker loses 20 hit per 2 tiles when attacking from 3 or more tiles away (not ballistae) |
| [anima_weapon_triangle](anima_weapon_triangle/) | Fire > Wind/Ice > Thunder > Fire tome triangle (+3 damage / +15 hit) from an editable table |
| [battle_max_damage](battle_max_damage/) | Configurable per-hit damage cap (up to 127) for real hits, the Silencer and the battle forecast |
| [s_rank_weapon_no_weight](s_rank_weapon_no_weight/) | Weapons the unit has S rank in weigh nothing for attack speed |
| [modular_staff_exp](modular_staff_exp/) | Fixed per-staff EXP from an editable table (unlisted staves keep vanilla EXP) |
| [limited_shop_items](limited_shop_items/) | Per-chapter limited shop stock with a stock column, kept through suspend without save changes |
| [goal_escape](goal_escape/) | Escape chapter goal (goal type 6): units that Wait on per-chapter escape tiles leave the map, and the chapter is won when all have escaped |
| [map_menu_character_biographies](map_menu_character_biographies/) | Biography command in the map menu: a scrolling character list that plays up to four CG text pages per character |
| [language_select](language_select/) | Flag-grid language selector after the boot logos, saved in the global save header, with an optional text-ID switch for translations (uses 4 bytes EWRAM) |
| [chatlog](chatlog/) | Conversation log for vanilla dialogue: SELECT opens a scrollback log of the chapter's dialogue (with text_engine, use its `TEXT_ENGINE_CHATLOG` instead) (uses 0x600 bytes EWRAM) |

## Adding a new patch

Use the project skill **extract-standalone-feature** (`.agents/skills/extract-standalone-feature/SKILL.md`).

Reference implementation: `two_random_number_growths/`.
