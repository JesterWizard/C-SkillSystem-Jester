#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"

const struct KernelDesigerConfig gKernelDesigerConfig = {
	.ai_dance_use = true, // false = AI never dances
	.ai_player_phase = false,
	.alpha_blend_movement_sprites = true, // Lex Talionis faded unit ghost while pathfinding
	.anima_weapon_triangle = false,
	.apply_dynamic_nosferatu_battle_anim = true, // apply Nosferatu anim to new spells
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
	.c03_do_not_flush_efx_status = true, // keep status palette flashes through C03 (attack start); false = vanilla wipe of ids matching 0xC
	.calculate_map_range_faster = true, // AI only ranges the 3 closest enemies
	.casual_mode = false,
	.chapter_names = true,
	.chatlog_enabled = true,
	.collect_dead_units = true,
	.custom_battle_quotes = true,
	.custom_character_animations = true,
	.custom_defeat_quotes = true,
	.custom_droppable_item_icon = true,
	.custom_fog_sight = true,
	.custom_guide = true,
	.custom_menu_options = true,
	.custom_staff_accuracy = 100,
	.custom_staves = true,
	.custom_support_conversations = true,
	.custom_talk_icon = true,
	.death_dance = true,
	.debug_autoload_skills = false,
	.deny_stat_screen_access = true,
	.display_status_flash_on_hit = true, // banim status flash from ext-hit, not ANIM_ROUND_POISON
	.display_terrain_bonuses_in_stat_screen = true,
	.dynamic_weapon_slots = true, // true = remap rank slots via gClassWeaponSlotConf; false = vanilla
	.enemy_can_combo_attack = false,
	.enemy_fog_vision = true,
	.engage_combo_attack = true, // requires SID_ChainAttack
	.expanded_hp = true,
	.fast_map_animations = false, // broken with free movement
	.fe7_mode_select = true,
	.fe8_rewritten_specific_changes = true,
	.flipped_enemy_sprites = true,
	.forge_mechanic = false,
	.fourth_allegiance = true,
	.free_movement = true,
	.gaiden_magic = true,
	.gaiden_magic_ai_use = false, // false = AI can't use gaiden magic
	.gaiden_magic_must_be_magic = false, // true = magic weapons only
	.gaiden_magic_requires_wrank = false, // true = require weapon rank
	.gaiden_magic_skill_extensions = true, // false = extra gaiden magic from an external skill table
	.gameover_quotes = false,
	.gen_new_scroll = false, // true = replace equipped skill and generate a scroll, false = learn for prep
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
	.lights_out_game = false,
	.limited_shop_items = true,
	.load_skills_through_feb_formatted_list = false, // FEBuilder-only skill list loading
	.lvup_mode_easy = 0, // 0=vanilla, 1=random, 2=fixed, 3=100%, 4=0%
	.lvup_mode_hard = 0, // 0=vanilla, 1=random, 2=fixed, 3=100%, 4=0%
	.lvup_mode_normal = 0, // 0=vanilla, 1=random, 2=fixed, 3=100%, 4=0%
	.map_menu_character_biographies = false,
	.max_color_backgrounds = true,
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
	.portrait_32_color = false,
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
	.real_time_battle = false, // broken; enemies act on a timer during player phase
	.real_time_enemy_interval_frames = 60, // 60 frames ≈ 1s between enemy action attempts
	.real_time_refresh_frames = 60 * 30, // soft refresh / cooldown clear every 30s
	.remove_move_path = false,
	.res_terrain_window = true,
	.rescue_drop_ai_use = true,
	.rescue_drop_move_again = true,
	.reset_bwl_stats_each_chapter = true,
	.restore_hp_on_level_up = false,
	.s_rank_weapon_no_weight = true,
	.send_inventory_on_death = true,
	.shield_ext_equip_config_en = false,
	.shield_system = false,
	.show_cgs_like_fe7 = true,
	.show_heal_amount = true,
	.show_true_2rn = true, // Options menu: show true 2RN hit rates
	.show_tutorial = 1, // 0=off, 1=easy mode only, 2=always
	.skill_shop = false,
	.skill_sub_menu_width = 10,
	.skill_tree = false,
	.skip_intro = false,
	.start_map_effects = false,
	.stat_gain_frame_speed = 10,
	.stat_page_gaiden_magic = true,
	.stat_page_personal_info = false,
	.stat_page_promotions = true,
	.stat_page_skill_style = 0, // 0=named list, 1=unnamed grid
	.stat_page_skill_tree = false,
	.stat_screen_allegiance_colors = true,
	.stat_screen_growths = 2, // 1=letter growths, 2=bonus growths in green
	.summons_gain_exp = false,
	.support_rewards = true,
	.talk_conversation_exp_reward = 10,
	.talk_on_level_up = true,
	.tellius_skill_capacity_base = 50,
	.tellius_skill_capacity_promoted = 25,
	.tellius_skill_capacity_system = true,
	.text_box_extension_layout = 2, // 0=vanilla, 1=extended 5-line, 2=paginated
	.two_random_number_growths = false,
	.unit_page_style = 1, // 1=with BWL, 2=with Leadership
	.unit_selection_quotes = true,
	.unlock_all_supports = true,
	.use_chinese_character = false,
	.variable_unit_descriptions = false,
	.vesly_achievements = false,
	.vesly_credits_cgs = true,
	.vesly_custom_ui = true, // ~200KB per style (~600KB so far)
	.vesly_danger_bones = false,
	.vesly_debugger = true,
	.vesly_draw_animations = true,
	.vesly_fast_forward_battle_animations = false,
	.vesly_notification_window = false,
	.vesly_reclass = false,
	.vesly_support_after_battle = true,
	.vesly_support_after_battle_combat_rate = 5,
	.vesly_support_after_battle_dance_rate = 10,
	.vesly_support_after_battle_kill_rate = 10,
	.vesly_support_after_battle_staff_rate = 10,
	.voice_acted_dialogue = true,
	.world_map_thought_bubbles = true,
	.wrank_bonux_rtext_auto_gen = true,
};
