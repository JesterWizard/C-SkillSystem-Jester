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
| [promotion_on_max_level](promotion_on_max_level/) | Unpromoted units promote when the map level-up window closes at level 20 |
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

## Adding a new patch

Use the project skill **extract-standalone-feature** (`.agents/skills/extract-standalone-feature/SKILL.md`).

Reference implementation: `two_random_number_growths/`.
