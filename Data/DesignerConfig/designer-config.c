#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"

const struct KernelDesigerConfig gKernelDesigerConfig = {

	.equip_skill_en = true,

	/* JESTER - This doesn't actually work right now, amend later */
	/**
	 * Deside whether to allow players to generate a new skill scroll,
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
	.voice_acted_dialogue = true,
	.forge_mechanic = false,
	.modular_staff_exp = true,

	.guaranteed_lvup = false,
	.lvup_mode_tutorial = CONFIG_LVUP_MODE_TUTORIAL,
	.lvup_mode_normal   = CONFIG_LVUP_MODE_NORMAL,
	.lvup_mode_hard	    = CONFIG_LVUP_MODE_HARD,

	.unit_page_style = CONFIG_PAGE1_WITH_BWL,
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
	.enemy_can_combo_attack = false, // true
	.menu_skill_disp_msg_en_n = CONFIG_MENU_SKILL_DISP_MSG_EN_N,
	.banim_switcher_en = false, // true
	.max_level = UNIT_LEVEL_MAX_RE,
	.max_level_record = UNIT_RECORDED_LEVEL_MAX,
};
