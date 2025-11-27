#include "common-chax.h"
#include "debuff.h"
#include "map-task.h"
#include "kernel-lib.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "constants/skills.h"

LYN_REPLACE_CHECK(PutUnitSpriteIconsOam);
void PutUnitSpriteIconsOam(void)
{
	int i;
	bool icon_blinking;

	/* Cache talkee id once to avoid repeated expensive GetTalkee() calls inside the loop */
#ifdef CONFIG_DISPLAY_TALK_ICON
	int cached_talkee_id = 0;
	bool have_cached_talkee = false;
	if (gBmSt.gameStateBits & BM_FLAG_1) {
		cached_talkee_id = GetTalkee(gActiveUnit);
		have_cached_talkee = true;
	}
#endif

	if (CheckFlag(0x84) != 0)
		return;

	icon_blinking = !((GetGameClock() & 0x3F) >= 0x28);

	PutChapterMarkedTileIconOam();

	for (i = 1; i <= 0xC0; i++) {
		int ix, iy, status;
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit) || unit->state & US_HIDDEN)
			continue;

		// GetUnitSpriteHideFlag
		if (!unit->pMapSpriteHandle || (unit->pMapSpriteHandle->config & 0x80))
			continue;

		ix = unit->xPos * 16 - gBmSt.camera.x;
		iy = unit->yPos * 16 - gBmSt.camera.y;

		if (ix < -16 || ix > DISPLAY_WIDTH)
			continue;

		if (iy < -16 || iy > DISPLAY_HEIGHT)
			continue;

#if defined(SID_MineDetector) && (COMMON_SKILL_VALID(SID_MineDetector))
		if (SkillTester(gActiveUnit, SID_MineDetector) && gBmSt.gameStateBits & BM_FLAG_1)
		{
			struct Trap* trap;

			for (int i = 0; i < ARRAY_COUNT_RANGE5x5; i++) 
			{
				int _x = gActiveUnit->xPos + gVecs_5x5[i].x;
				int _y = gActiveUnit->yPos + gVecs_5x5[i].y;

				trap = GetTrapAt(_x, _y);

				if (trap->type == TRAP_MINE)
				{
					MapTaskVec.x = _x  * 16 - gBmSt.camera.x;
					MapTaskVec.y = _y  * 16 - gBmSt.camera.y;
					MapTaskPutOamHi(MTSKCONF_WARNING, OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0xEE0 / 0x20));	
				}
			}
		}
#endif

		MapTaskVec.x = ix;
		MapTaskVec.y = iy;

#ifdef CONFIG_DISPLAY_TALK_ICON
		if (have_cached_talkee && cached_talkee_id == UNIT_CHAR_ID(unit)) {
			MapTaskPutOamHi(MTSKCONF_TALK_1, OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0xB00 / 0x20));
			MapTaskPutOamHi(MTSKCONF_TALK_2, OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0xF00 / 0x20));
		}
#endif

		/**
		 * 1. HpBar
		 */
		if (unit->curHP != unit->maxHP) {
			int _cur = unit->curHP;
			int _max = unit->maxHP;

			if (_cur > _max)
				_cur = _max;

			MapTaskPutOamHi(
				MTSKCONF_HPBAR,
				OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(gpHpBarChr[k_udiv(_cur * 10, _max)]));
		}

		/**
		 * 2. Check on blinking icon
		 */
		if (icon_blinking) {
			/**
			 * 3. Debuff icon
			 */
			status = GetUnitStatusIndex(unit);

			if (status != UNIT_STATUS_NONE) {
				const struct DebuffInfo *info = &gpDebuffInfos[status];

				if (info->on_draw)
					info->on_draw(unit, ix, iy);
				else {
					bool statusIconApplied;

					switch (status) {
					case NEW_UNIT_STATUS_SLOW:
						MapTaskPutOamHi(MTSKCONF_STATUS_SLOW, OAM2_LAYER(2) + OAM2_CHR(0x300 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_BREAK:
						MapTaskPutOamHi(MTSKCONF_STATUS_BREAK, OAM2_LAYER(2) + OAM2_CHR(0x320 / 0x20));
						statusIconApplied = true;
						break;
						
					case NEW_UNIT_STATUS_TOXIC_POISON:
						MapTaskPutOamHi(MTSKCONF_STATUS_TOXIC, OAM2_LAYER(2) + OAM2_CHR(0x340 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_HIDE:
						MapTaskPutOamHi(MTSKCONF_STATUS_HIDE, OAM2_LAYER(2) + OAM2_CHR(0x360 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_DECOY:
						MapTaskPutOamHi(MTSKCONF_STATUS_DECOY, OAM2_LAYER(2) + OAM2_CHR(0x380 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_DOOM:
						MapTaskPutOamHi(MTSKCONF_STATUS_DOOM, OAM2_LAYER(2) + OAM2_CHR(0x3A0 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_REPLICATE:
						MapTaskPutOamHi(MTSKCONF_STATUS_REPLICATE, OAM2_LAYER(2) + OAM2_CHR(0x3A0 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_ARMOR_EFFECTIVE:
					case NEW_UNIT_STATUS_CAVALRY_EFFECTIVE:
					case NEW_UNIT_STATUS_FLIER_EFFECTIVE:
					case NEW_UNIT_STATUS_INFANTRY_EFFECTIVE:
						MapTaskPutOamHi(MTSKCONF_STATUS_EFFECTIVE, OAM2_LAYER(2) + OAM2_CHR(0x3C0 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_SHOCK:
						MapTaskPutOamHi(MTSKCONF_STATUS_SHOCK, OAM2_LAYER(2) + OAM2_CHR(0x3E0 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_PANIC:
						MapTaskPutOamHi(MTSKCONF_STATUS_REPLICATE, OAM2_LAYER(2) + OAM2_CHR(0x700 / 0x20));
						statusIconApplied = true;
						break;

					case NEW_UNIT_STATUS_ENFEEBLE:
						MapTaskPutOamHi(MTSKCONF_STATUS_REPLICATE, OAM2_LAYER(2) + OAM2_CHR(0x720 / 0x20));
						statusIconApplied = true;
						break;

					// case NEW_UNIT_STATUS_PROVIDENCE:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_PROVIDENCE, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;

					// case NEW_UNIT_STATUS_BOUND:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_BOUND, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;

					// case NEW_UNIT_STATUS_HEX:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_HEX, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;

					// case NEW_UNIT_STATUS_QUICKEN:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_QUICKEN, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;

					// case NEW_UNIT_STATUS_DELAY:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_DELAY, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;

					// case NEW_UNIT_STATUS_RENEWAL:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_RENEWAL, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;

					// case NEW_UNIT_STATUS_INVEST:
					// 	MapTaskPutOamHi(MTSKCONF_STATUS_RENEWAL, OAM2_LAYER(2) + OAM2_CHR(0x000 / 0x20));
					// 	statusIconApplied = true;
					// 	break;
					
					}

					if (!statusIconApplied)
					{
						switch (info->positive_type) {
						case STATUS_DEBUFF_NEGATIVE:
							MapTaskPutOamHi(MTSKCONF_COMM_DEBUFF, OAM2_LAYER(2) + OAM2_CHR(0x980 / 0x20));
							break;

						case STATUS_DEBUFF_POSITIVE:
							MapTaskPutOamHi(MTSKCONF_COMM_BUFF, OAM2_LAYER(2) + OAM2_CHR(0xD80 / 0x20));
							break;
						}
					}
				}
			} else {
				switch (SimulateStatDebuffPositiveType(unit)) {
				case STATUS_DEBUFF_NEGATIVE:
					MapTaskPutOamHi(MTSKCONF_COMM_DEBUFF, OAM2_LAYER(2) + OAM2_CHR(0x980 / 0x20));
					break;

				case STATUS_DEBUFF_POSITIVE:
					MapTaskPutOamHi(MTSKCONF_COMM_BUFF, OAM2_LAYER(2) + OAM2_CHR(0xD80 / 0x20));
					break;
				}
			}

			/**
			 * 4. Misc
			 */
			if (unit->state & US_RESCUING) {
				static const u16 pal_idx[] = { 0xC, 0xE, 0xD };
				int faction = unit->rescue >> 6;

				MapTaskPutOamHi(
					MTSKCONF_RESCUING,
					OAM2_PAL(pal_idx[faction]) + OAM2_LAYER(2) + OAM2_CHR(0x60 / 0x20));
			} else if ((UNIT_FACTION(unit) != FACTION_BLUE) && (UNIT_CATTRIBUTES(unit) & CA_BOSS)) {
				MapTaskPutOamHi(
					MTSKCONF_BOSS,
					OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0x200 / 0x20));
			} else if (UNIT_CHAR_ID(unit) == GetROMChapterStruct(gPlaySt.chapterIndex)->protectCharacterIndex) {
				MapTaskPutOamHi(
					MTSKCONF_PROTECTION,
					OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0x220 / 0x20));
			}

#ifdef CONFIG_DISPLAY_DROPPABLE_ITEM_ICON
			if (unit->state & US_DROP_ITEM)
   				MapTaskPutOamHi(MTSKCONF_DROP_ITEM, OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0xC80 / 0x20));	
#endif
		}
	}
}
