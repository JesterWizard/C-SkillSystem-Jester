#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"

const struct KernelDesigerConfig gKernelDesigerConfig = {

	/* JESTER - This doesn't actually work right now, amend later */
	/**
	 * Decide whether to allow players to generate a new skill scroll,
	 * if he try to use skill scroll to a unit who has already filled with skills.
	 *
	 * 1: choose a equipped skill to replace
	 * 0: just learn the skill (equippable in prep-skill screen)
	 */
	.gen_new_scroll = false,
	.max_equipable_skill = UNIT_RAM_SKILLS_LEN,
	.remove_move_path = false,
	.use_chinese_character = false,
	.show_tutorial = 1, /* 0: No kernel tutorial, 1: Only show kernel tutorial in easy mode, 2: Show kernel tutorial anytime */
	.engage_combo_attack = true, // Requires the skill SID_ChainAttack
	.battle_surrend_en = false,
	.hit_decrease_on_range = false,
	.debug_autoload_skills = false,

	.casual_mode = false,
#ifdef CONFIG_VOICE_ACTED_DIALOGUE
	.voice_acted_dialogue = true,
#endif
	.forge_mechanic = false,
	.modular_staff_exp = true,
	.arena_show_opponent_in_advance = false,
	.arena_let_player_use_upgraded_weapons = true,
	.arena_calculate_weapon_based_on_level = true,
	.talk_on_level_up = true,
	.death_dance = true,
	.s_rank_weapon_no_weight = true,
	.display_terrain_bonuses_in_stat_screen = true,
	.show_heal_amount = true,
	.unlock_all_supports = true,
	.promotion_on_max_level = true,
	.stat_screen_growths = 2, // 1 = Growths as letters, 2 = bonus growths as green
	.restore_hp_on_level_up = false,
	.auto_repair_weapons = false,
	.custom_support_conversations = true,
	.custom_fog_sight = true,
	.custom_talk_icon = true,
	.custom_droppable_item_icon = true,
	.l_button_same_faction_cycling = true,
	.two_random_number_growths = false,
	.max_simultaneous_support_conversations = 10,
	.kill_rewards = true,
	.menu_option_refuge = false,
	.promote_enemy_on_kill = true,
	.promote_enemy_boost = 3,
	.no_wait_after_trading = true,
	.send_inventory_on_death = true,
	.biorhythm_mechanic = false,
	.skip_intro = false,
	.base_chapters = false,
	.fast_map_animations = false, // This is funny when free movement is enabled, might fix later
	.skill_shop = false,
	.stat_gain_frame_speed = 10,
	.deny_stat_screen_access = true,
	.support_rewards = true,
	.konami_style_bonus_screen = true,
	.battle_max_damage = 127,
	.ignore_stop_on_petrify_sleep = true,
	.leadership = true,
	.variable_unit_descriptions = false,
	.anima_weapon_triangle = false,
	.stat_page_skill_style = 0, // 0 = Display skills in list with names, 1 = Display skills in grid without names,
	.stat_page_gaiden_magic = true,
	.stat_page_personal_info = false,
	.stat_page_promotions = true,
	.summons_gain_exp = false,
	.collect_dead_units = true,
	.goal_timer = false,
	.goal_escape = false,
	.prep_menu_skills = true,
	.prep_menu_augury = true,
	.prep_menu_infuse = false,
	.prep_menu_bexp = true,
	.prep_menu_base_conversations = false,
	.prep_menu_event_replay = false,
	.expanded_hp = true,
	.vesly_debugger = true,
	.map_menu_character_biographies = false,
	.custom_staff_accuracy = 100,
	.limited_shop_items = true,
	.flipped_enemy_sprites = true,
	.custom_battle_quotes = true,
	.custom_defeat_quotes = true,
	.mp_system = true,
	.laguz_bars = false,
	.reset_bwl_stats_each_chapter = true,
	.quality_of_life_fixes = true,
	.guaranteed_lvup = false,
	.lvup_mode_easy     = 0,   /* 0: vanilla, 1: random, 2: fixed, 3: 100%, 4: 0% */
	.lvup_mode_normal   = 0,   /* 0: vanilla, 1: random, 2: fixed, 3: 100%, 4: 0% */
	.lvup_mode_hard	    = 0,   /* 0: vanilla, 1: random, 2: fixed, 3: 100%, 4: 0% */
	.unit_page_style = 1, // 1 = With BWL, 2 = With Leadership
	.gaiden_magic = true,
	.gaiden_magic_must_be_magic = false,    // False = all weapons are allowed, True = Only magic weapons
	.gaiden_magic_requires_wrank = false,   // False = Doesn't require unit to have a rank in that weapon to use
	.gaiden_magic_ai_use = false,           // False = AI can't use gaiden magic
	.gaiden_magic_skill_extensions = true, // False = Apply an extenal table with skills that apply more gaiden magic
	.no_suspend_in_aiphase = false,
	.shield_system = false,
	.shield_ext_equip_config_en = false,
	.auto_narrow_font = true,
	.skill_sub_menu_width = 10,
	.wrank_bonux_rtext_auto_gen = true,
	.enemy_can_combo_attack = false,
	.banim_switcher_en = false,
	.max_level = UNIT_LEVEL_MAX_RE,
	.max_level_record = UNIT_RECORDED_LEVEL_MAX,
	.tellius_skill_capacity_system = true,
	.tellius_skill_capacity_base = 50,
	.tellius_skill_capacity_promoted = 25,
	.calculate_map_range_faster = true, // Speeds up AI decision making by only calculating the three closest enemies, without it the AI takes forever 
	.custom_character_animations = true,
	.apply_dynamic_nosferatu_battle_anim = true, // This exists to allow the Nosferatu battle animation to be dynamically applied to new spells
	.display_status_flash_on_hit = true, // In banim, unit status flashing is decided by ext-hit rather than ANIM_ROUND_POISON
	.c03_do_not_flush_efx_status = true, // Honestly, no clue. but Mokha used it so I'm moving it here
	.load_skills_through_feb_formatted_list = false, // Honestly, this one only means anything if you're using FEBuilder
	.item_effect_revamp = true,
	.vesly_danger_bones = false,
	.vesly_support_after_battle = true,
	.vesly_support_after_battle_kill_rate = 10,
	.vesly_support_after_battle_combat_rate = 5,
	.vesly_support_after_battle_dance_rate = 10,
	.vesly_support_after_battle_staff_rate = 10,
	.vesly_fast_forward_battle_animations = true,
	.vesly_extended_help_boxes = true,
	.free_movement = true,
	.gameover_quotes = true,
	.vesly_credits_cgs = true,
	.fe8_rewritten_specific_changes = true,
	.arena_limits = false,
	.arena_roster_menu = true,
	.vesly_custom_ui = true, // About 200KB per style (600KB so far)
	.custom_menu_options = true,
	.multiple_fog_stages = true,
	.custom_staves = true,
	.menu_skill_ai_use = false,
	.rescue_drop_ai_use = true,
	.start_map_effects = false,
	.world_map_thought_bubbles = true,
	.ai_player_phase = false,
	.prestige = true,
	.promote_trainees_in_chapter = true,
	.rescue_drop_move_again = true,
	.talk_conversation_exp_reward = 10,
	.infinite_durability = true,
};
