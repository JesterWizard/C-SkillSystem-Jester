#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"

const struct KernelDesigerConfig gKernelDesigerConfig = {

	.equip_skill_en = true,

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
	.kernel_tutorial_level = CONFIG_KTUT_LEVEL,
	.combo_attack_en = true,
	.battle_surrend_en = false,
	.hit_decrease_on_range = false,
	.debug_autoload_skills = false,

	.casual_mode = true,
#ifdef CONFIG_VOICE_ACTED_DIALOGUE
	.voice_acted_dialogue = true,
#endif
	.forge_mechanic = false,
	.modular_staff_exp = true,
	.arena_show_opponent_in_advance = true,
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
	.restore_hp_on_level_up = true,
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
	.base_chapters = true,
	.fast_map_animations = false, // This is funny when free movement is enabled, might fix later
	.skill_points_engage = false,
	.stat_gain_frame_speed = 10,
	.deny_stat_screen_access = true,
	.support_rewards = true,
	.konami_style_bonus_screen = true,
	.battle_max_damage = 127,
	.ignore_stop_on_petrify_sleep = true,
	.leadership = true,
	.variable_unit_descriptions = false,
	.anima_weapon_triangle = false,
	
	.stat_page_gaiden_magic = false,
	.stat_page_personal_info = true,
	.stat_page_promotions = true,

	.guaranteed_lvup = false,
	.lvup_mode_tutorial = CONFIG_LVUP_MODE_TUTORIAL,
	.lvup_mode_normal   = CONFIG_LVUP_MODE_NORMAL,
	.lvup_mode_hard	    = CONFIG_LVUP_MODE_HARD,

	.unit_page_style = 1, // 1 = With BWL, 2 = With Leadership
	.skil_page_style = CONFIG_PAGE4_MOKHA_PLAN_B,

#ifdef CONFIG_USE_GAIDEN_MAGIC
	.gaiden_magic_en = true,
	.gaiden_magic_must_be_magic = CONFIG_GAIDEN_MAGIC_MUST_BE_MAGIC,
	.gaiden_magic_requires_wrank = CONFIG_GAIDEN_MAGIC_REQUIRES_WRANK,
	.gaiden_magic_ai_en = CONFIG_GAIDEN_MAGIC_AI_EN,
	.gaiden_magic_ext_conf_en = CONFIG_GAIDEN_EXT_CONF_EN,
#endif

	.no_suspend_in_aiphase = true,

#ifdef CONFIG_INSTALL_KERNEL_SHIELD
	.shield_en = true,
	.shield_ext_equip_config_en = true,
#endif

	.auto_narrow_font = true,
	.skill_sub_menu_width = 10,
	.wrank_bonux_rtext_auto_gen = true,
	.enemy_can_combo_attack = false,
	.menu_skill_disp_msg_en_n = CONFIG_MENU_SKILL_DISP_MSG_EN_N,
	.banim_switcher_en = false,
	.max_level = UNIT_LEVEL_MAX_RE,
	.max_level_record = UNIT_RECORDED_LEVEL_MAX,
};
