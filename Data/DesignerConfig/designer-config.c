#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"

const struct KernelDesigerConfig gKernelDesigerConfig = {
	.ai_dance_use = true, // False = AI never uses the dance command
	.ai_player_phase = false,
	.anima_weapon_triangle = false,
	.apply_dynamic_nosferatu_battle_anim = true, // This exists to allow the Nosferatu battle animation to be dynamically applied to new spells
	.arena_calculate_weapon_based_on_level = true,
	.arena_let_player_use_upgraded_weapons = true,
	.arena_limits = false,
	.arena_roster_menu = false,
	.arena_show_opponent_in_advance = true,
	.auto_narrow_font = true,
	.auto_repair_weapons = false,
	.banim_switcher_en = false,
	.base_chapters = false,
	.battle_max_damage = 127,
	.battle_surrend_en = false,
	.biorhythm_mechanic = false,
	.c03_do_not_flush_efx_status = true, // Honestly, no clue. but Mokha used it so I'm moving it here
	.calculate_map_range_faster = true, // Speeds up AI decision making by only calculating the three closest enemies, without it the AI takes forever 
	.casual_mode = false,
	.chatlog_enabled = true,
	.collect_dead_units = true,
	.custom_battle_quotes = true,
	.custom_character_animations = true,
	.custom_defeat_quotes = true,
	.custom_droppable_item_icon = true,
	.custom_fog_sight = true,
	.custom_menu_options = true,
	.custom_staff_accuracy = 100,
	.custom_staves = true,
	.custom_support_conversations = true,
	.custom_talk_icon = true,
	.death_dance = true,
	.debug_autoload_skills = false,
	.deny_stat_screen_access = true,
	.display_status_flash_on_hit = true, // In banim, unit status flashing is decided by ext-hit rather than ANIM_ROUND_POISON
	.display_terrain_bonuses_in_stat_screen = true,

	/**
	 * When true, classes may remap the eight Unit::ranks slots onto any
	 * rank-bearing weapon types via gClassWeaponSlotConf (knives, guns, etc.).
	 * When false, slot N always stores weapon type N (vanilla behaviour).
	 */
	.dynamic_weapon_slots = true,
	.enemy_can_combo_attack = false,
	.enemy_fog_vision = true,
	.engage_combo_attack = true, // Requires the skill SID_ChainAttack
	.expanded_hp = true,
	.fast_map_animations = false, // This is funny when free movement is enabled, might fix later
	.fe8_rewritten_specific_changes = true,
	.flipped_enemy_sprites = true,
	.forge_mechanic = false,
	.free_movement = true,
	.gaiden_magic = true,
	.gaiden_magic_ai_use = false,           // False = AI can't use gaiden magic
	.gaiden_magic_must_be_magic = false,    // False = all weapons are allowed, True = Only magic weapons
	.gaiden_magic_requires_wrank = false,   // False = Doesn't require unit to have a rank in that weapon to use
	.gaiden_magic_skill_extensions = true, // False = Apply an extenal table with skills that apply more gaiden magic
	.gameover_quotes = false,

	/* JESTER - This doesn't actually work right now, amend later */
	/**
	 * Decide whether to allow players to generate a new skill scroll,
	 * if he try to use skill scroll to a unit who has already filled with skills.
	 *
	 * 1: choose a equipped skill to replace
	 * 0: just learn the skill (equippable in prep-skill screen)
	 */
	.gen_new_scroll = false,
	.goal_escape = false,
	.goal_timer = false,
	.guaranteed_lvup = false,
	.half_body_portraits = false,
	.hit_decrease_on_range = false,
	.ignore_stop_on_petrify_sleep = true,
	.infinite_durability = true,
	.item_effect_revamp = true,
	.kill_rewards = true,
	.konami_style_bonus_screen = true,
	.l_button_same_faction_cycling = true,
	.laguz_bars = false,
	.leadership = true,
	.limited_shop_items = true,
	.load_skills_through_feb_formatted_list = false, // Honestly, this one only means anything if you're using FEBuilder
	.lvup_mode_easy     = 0,   /* 0: vanilla, 1: random, 2: fixed, 3: 100%, 4: 0% */
	.lvup_mode_hard	    = 0,   /* 0: vanilla, 1: random, 2: fixed, 3: 100%, 4: 0% */
	.lvup_mode_normal   = 0,   /* 0: vanilla, 1: random, 2: fixed, 3: 100%, 4: 0% */
	.map_menu_character_biographies = false,
	.max_equipable_skill = UNIT_RAM_SKILLS_LEN,
	.max_level = UNIT_LEVEL_MAX_RE,
	.max_level_record = UNIT_RECORDED_LEVEL_MAX,
	.max_simultaneous_support_conversations = 10,
	.menu_option_refuge = false,
	.menu_skill_ai_use = false,
	.modular_staff_exp = true,
	.mokha_aoe_enabled = true,
	.mp_system = true,
	.multiple_fog_stages = true,
	.no_suspend_in_aiphase = false,
	.no_wait_after_trading = true,
	.pair_up_enabled = true,
	.prep_menu_augury = true,
	.prep_menu_base_conversations = false,
	.prep_menu_bexp = true,
	.prep_menu_event_replay = false,
	.prep_menu_infuse = false,
	.prep_menu_skills = true,
	.prestige = true,
	.promote_enemy_boost = 3,
	.promote_enemy_on_kill = true,
	.promote_trainees_in_chapter = true,
	.promotion_on_max_level = true,
	.quality_of_life_fixes = true,

	/**
	 * Real-time battle mode.
	 * When enabled, the map stays on player phase and enemies act on a timer.
	 * Interval is in frames (GBA runs at 60 FPS, so 60 ~= 1 second).
	 */
	.real_time_battle = false, // Broken mess, do not use
	.real_time_enemy_interval_frames = 60,  /* 1 second between enemy action attempts */
	.real_time_refresh_frames = 60 * 30,    /* soft refresh / cooldown clear every 30s */
	.remove_move_path = false,
	.rescue_drop_ai_use = true,
	.rescue_drop_move_again = true,
	.reset_bwl_stats_each_chapter = true,
	.restore_hp_on_level_up = false,
	.s_rank_weapon_no_weight = true,
	.send_inventory_on_death = true,
	.shield_ext_equip_config_en = false,
	.shield_system = false,
	.show_heal_amount = true,
	.show_true_2rn = true, /* Options menu: show true 2RN hit rates */
	.show_tutorial = 1, /* 0: No kernel tutorial, 1: Only show kernel tutorial in easy mode, 2: Show kernel tutorial anytime */
	.skill_shop = false,
	.skill_sub_menu_width = 10,
	.skill_tree = false,
	.skip_intro = false,
	.start_map_effects = false,
	.stat_gain_frame_speed = 10,
	.stat_page_gaiden_magic = true,
	.stat_page_personal_info = false,
	.stat_page_promotions = true,
	.stat_page_skill_style = 0, // 0 = Display skills in list with names, 1 = Display skills in grid without names,
	.stat_page_skill_tree = false,
	.stat_screen_growths = 2, // 1 = Growths as letters, 2 = bonus growths as green
	.summons_gain_exp = false,
	.support_rewards = true,
	.talk_conversation_exp_reward = 10,
	.talk_on_level_up = true,
	.tellius_skill_capacity_base = 50,
	.tellius_skill_capacity_promoted = 25,
	.tellius_skill_capacity_system = true,
	.text_box_extension_layout = 2, /* 0=vanilla, 1=extended 5-line, 2=paginated */

	/**
	 * Lex Talionis: while pathfinding, show a faded moving-map-sprite
	 * ghost of the active unit at the cursor. Gone after A or B.
	 */
	.translucent_unit_sprite = true,
	.two_random_number_growths = false,
	.unit_page_style = 1, // 1 = With BWL, 2 = With Leadership
	.unlock_all_supports = true,
	.use_chinese_character = false,
	.variable_unit_descriptions = false,
	.vesly_achievements = false,
	.vesly_credits_cgs = true,
	.vesly_custom_ui = true, // About 200KB per style (600KB so far)
	.vesly_danger_bones = false,
	.vesly_debugger = true,
	.vesly_fast_forward_battle_animations = false,
	.vesly_notification_window = false,
	.vesly_support_after_battle = true,
	.vesly_support_after_battle_combat_rate = 5,
	.vesly_support_after_battle_dance_rate = 10,
	.vesly_support_after_battle_kill_rate = 10,
	.vesly_support_after_battle_staff_rate = 10,
#ifdef CONFIG_VOICE_ACTED_DIALOGUE
	.voice_acted_dialogue = true,
#endif
	.world_map_thought_bubbles = true,
	.wrank_bonux_rtext_auto_gen = true,
};
