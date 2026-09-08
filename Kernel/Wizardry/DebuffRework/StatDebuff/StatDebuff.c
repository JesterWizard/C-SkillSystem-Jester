#include "common-chax.h"
#include "kernel-lib.h"
#include "debuff.h"
#include "skill-system.h"
#include "constants/skills.h"

#define LOCAL_TRACE 0

inline struct StatDebuffStatus *GetUnitStatDebuffStatus(struct Unit *unit)
{
	if (unit == &gBattleActor.unit)
		return &sStatDebuffStatusBattleUnit[0];
	else if (unit == &gBattleTarget.unit)
		return &sStatDebuffStatusBattleUnit[1];

	return gpStatDebuffStatusPool[unit->index & 0xFF];
}

int SimulateStatDebuffPositiveType(struct Unit *unit)
{
	bool has_status = false;
	int i, positive = 0;
	struct StatDebuffStatus *stat;

	if (!UNIT_IS_VALID(unit))
		return STATUS_DEBUFF_NONE;

	stat = GetUnitStatDebuffStatus(unit);
	switch (stat->st.bitfile.is_buff_chk) {
	case STATUS_DEBUFF_NEGATIVE:
	case STATUS_DEBUFF_POSITIVE:
		return stat->st.bitfile.is_buff_chk;

	case STATUS_DEBUFF_NONE_NO_CALC:
		return STATUS_DEBUFF_NONE;

	case STATUS_DEBUFF_NONE:
	default:
		break;
	}

	for (i = UNIT_STAT_DEBUFF_IDX_START; i < UNIT_STAT_DEBUFF_MAX; i++) {
		if (!_BIT_CHK(stat->st.bitmask, i))
			continue;

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
        if (SkillTester(unit, SID_Contrary))
        {
            switch (gpStatDebuffInfos[i].positive_type)
            {
            case STATUS_DEBUFF_NEGATIVE:
                positive++;
                break;

            case STATUS_DEBUFF_POSITIVE:
                positive++;
                break;

            case STATUS_DEBUFF_NONE:
            default:
                break;
            }
        }
        else
        {
            switch (gpStatDebuffInfos[i].positive_type)
            {
            case STATUS_DEBUFF_NEGATIVE:
                positive--;
                break;

            case STATUS_DEBUFF_POSITIVE:
                positive++;
                break;

            case STATUS_DEBUFF_NONE:
            default:
                break;
            }
        }

#else
        switch (gpStatDebuffInfos[i].positive_type)
        {
        case STATUS_DEBUFF_NEGATIVE:
            positive--;
            break;

        case STATUS_DEBUFF_POSITIVE:
            positive++;
            break;

        case STATUS_DEBUFF_NONE:
        default:
            break;
        }
#endif
	}

	if (positive < 0) 
	{
		stat->st.bitfile.is_buff_chk = STATUS_DEBUFF_NEGATIVE;
		return STATUS_DEBUFF_NEGATIVE;
	} 
	else if (positive > 0 || has_status) 
	{
		stat->st.bitfile.is_buff_chk = STATUS_DEBUFF_POSITIVE;
		return STATUS_DEBUFF_POSITIVE;
	}
    else
    {
        stat->st.bitfile.is_buff_chk = STATUS_DEBUFF_NONE_NO_CALC;
        return STATUS_DEBUFF_NONE;
    }
}

void ResetStatDebuffPositiveType(struct Unit *unit)
{
	if (unit == NULL)
		return;

	GetUnitStatDebuffStatus(unit)->st.bitfile.is_buff_chk = STATUS_DEBUFF_NONE;
}

void SetUnitStatDebuff(struct Unit *unit, enum UNIT_STAT_DEBUFF_IDX debuff)
{
	if (debuff >= UNIT_STAT_DEBUFF_MAX) {
		Errorf("ENOTDIR: %d", debuff);
		hang();
	}

	if (!UNIT_IS_VALID(unit) || debuff == 0)
		return;

	_BIT_SET(GetUnitStatDebuffStatus(unit)->st.bitmask, debuff);
	ResetStatDebuffPositiveType(unit);
}

void ClearUnitStatDebuff(struct Unit *unit, enum UNIT_STAT_DEBUFF_IDX debuff)
{
	if (debuff >= UNIT_STAT_DEBUFF_MAX) {
		Errorf("ENOTDIR: %d", debuff);
		hang();
	}

	if (!UNIT_IS_VALID(unit) || debuff == 0)
		return;

	_BIT_CLR(GetUnitStatDebuffStatus(unit)->st.bitmask, debuff);
	ResetStatDebuffPositiveType(unit);
}

bool CheckUnitStatDebuff(struct Unit *unit, enum UNIT_STAT_DEBUFF_IDX debuff)
{
	if (debuff >= UNIT_STAT_DEBUFF_MAX) {
		Errorf("ENOTDIR: %d", debuff);
		hang();
	}

	if (!UNIT_IS_VALID(unit) || debuff == 0)
		return false;

	return _BIT_CHK(GetUnitStatDebuffStatus(unit)->st.bitmask, debuff);
}

void MSU_SaveStatDebuff(u8 *dst, const u32 size)
{
#ifdef CONFIG_FOURTH_ALLEGIANCE
    if (size < (sizeof(sStatDebuffStatusAlly) + sizeof(sStatDebuffStatusEnemy) + sizeof(sStatDebuffStatusNpc) + sizeof(sStatDebuffStatusFourth)))
#else
    if (size < (sizeof(sStatDebuffStatusAlly) + sizeof(sStatDebuffStatusEnemy) + sizeof(sStatDebuffStatusNpc)))
#endif
    {
        Errorf("ENOMEM: %d", size);
        hang();
    }

    WriteAndVerifySramFast(
        sStatDebuffStatusAlly,
        dst,
        sizeof(sStatDebuffStatusAlly));

    dst += sizeof(sStatDebuffStatusAlly);

    WriteAndVerifySramFast(
        sStatDebuffStatusEnemy,
        dst,
        sizeof(sStatDebuffStatusEnemy));

    dst += sizeof(sStatDebuffStatusEnemy);

    WriteAndVerifySramFast(
        sStatDebuffStatusNpc,
        dst,
        sizeof(sStatDebuffStatusNpc));

#ifdef CONFIG_FOURTH_ALLEGIANCE
    dst += sizeof(sStatDebuffStatusNpc);
    
    WriteAndVerifySramFast(
        sStatDebuffStatusFourth,
        dst,
        sizeof(sStatDebuffStatusFourth));
#endif
}

void MSU_LoadStatDebuff(u8 *src, const u32 size)
{
#ifdef CONFIG_FOURTH_ALLEGIANCE
    if (size < (sizeof(sStatDebuffStatusAlly) + sizeof(sStatDebuffStatusEnemy) + sizeof(sStatDebuffStatusNpc) + sizeof(sStatDebuffStatusFourth)))
#else
    if (size < (sizeof(sStatDebuffStatusAlly) + sizeof(sStatDebuffStatusEnemy) + sizeof(sStatDebuffStatusNpc)))
#endif
    {
        Errorf("ENOMEM: %d", size);
        hang();
    }

    WriteAndVerifySramFast(
        src,
        sStatDebuffStatusAlly,
        sizeof(sStatDebuffStatusAlly));

    src += sizeof(sStatDebuffStatusAlly);

    WriteAndVerifySramFast(
        src,
        sStatDebuffStatusEnemy,
        sizeof(sStatDebuffStatusEnemy));

    src += sizeof(sStatDebuffStatusEnemy);

    WriteAndVerifySramFast(
        src,
        sStatDebuffStatusNpc,
        sizeof(sStatDebuffStatusNpc));

#ifdef CONFIG_FOURTH_ALLEGIANCE
    src += sizeof(sStatDebuffStatusNpc);

    WriteAndVerifySramFast(
        sStatDebuffStatusFourth,
        src,
        sizeof(sStatDebuffStatusFourth));
#endif
}

void TickUnitStatDebuff(struct Unit *unit, enum STATUS_DEBUFF_TICK_TYPE type)
{
	bool ticked = false;
	int i;
	u32 *bitfile = GetUnitStatDebuffStatus(unit)->st.bitmask;

	for (i = UNIT_STAT_DEBUFF_IDX_START; i < UNIT_STAT_DEBUFF_MAX; i++) {
		if (_BIT_CHK(bitfile, i) && type == gpStatDebuffInfos[i].tick_type) {
			ticked = true;
			_BIT_CLR(bitfile, i);
		}
	}

	if (ticked)
		ResetStatDebuffPositiveType(unit);
}

/**
 * Pre-battle calc
 */
void PreBattleCalcStatDebuffs(struct BattleUnit *bu, struct BattleUnit *defender)
{
	int i;
	u32 *bitfile = GetUnitStatDebuffStatus(&bu->unit)->st.bitmask;

	for (i = UNIT_STAT_DEBUFF_IDX_START; i < UNIT_STAT_DEBUFF_MAX; i++) {
		if (_BIT_CHK(bitfile, i)) {
			const struct DebuffInfo *info = &gpStatDebuffInfos[i];

			bu->battleAttack       += info->battle_status.atk;
			bu->battleDefense      += info->battle_status.def;
			bu->battleHitRate      += info->battle_status.hit;
			bu->battleAvoidRate    += info->battle_status.avo;
			bu->battleCritRate     += info->battle_status.crit;
			bu->battleSilencerRate += info->battle_status.silencer;
			bu->battleDodgeRate    += info->battle_status.dodge;
		}
	}
}

/**
 * Modular status-getter
 */

STATIC_DECLAR void GenerateStatDebuffMsgBufExt(struct Unit *unit, u32 *bitfile, struct StatDebuffMsgBuf *buf)
{
	int i;
	bool in_panic = false;

	memset(buf, 0, sizeof(*buf));

	buf->bitfile[0] = bitfile[0];
	buf->bitfile[1] = bitfile[1];
	buf->bitfile[2] = bitfile[2];
	buf->bitfile[3] = bitfile[3];

	buf->uid = unit->index;

	LTRACEF("pid=%#x, bitfile [%p]=%#lx", UNIT_CHAR_ID(unit), bitfile, *bitfile);

	if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_PANIC)
		in_panic = true;

	for (i = UNIT_STAT_DEBUFF_IDX_START; i < UNIT_STAT_DEBUFF_MAX; i++) {
		if (_BIT_CHK(bitfile, i)) {
			const struct DebuffInfo *info = &gpStatDebuffInfos[i];

			if (in_panic == true && info->positive_type == STATUS_DEBUFF_POSITIVE) {
				buf->pow -= info->unit_status.pow;
				buf->mag -= info->unit_status.mag;
				buf->skl -= info->unit_status.skl;
				buf->spd -= info->unit_status.spd;
				buf->def -= info->unit_status.def;
				buf->res -= info->unit_status.res;
				buf->lck -= info->unit_status.lck;
				buf->mov -= info->unit_status.mov;
              	buf->curHP -= info->unit_status.curHP;
                buf->maxHP -= info->unit_status.maxHP;
			} else {
				buf->pow += info->unit_status.pow;
				buf->mag += info->unit_status.mag;
				buf->skl += info->unit_status.skl;
				buf->spd += info->unit_status.spd;
				buf->def += info->unit_status.def;
				buf->res += info->unit_status.res;
				buf->lck += info->unit_status.lck;
				buf->mov += info->unit_status.mov;
              	buf->curHP -= info->unit_status.curHP;
                buf->maxHP -= info->unit_status.maxHP;
			}

			if (info->cannot_move == true)
				buf->special_mask |= SP_STAT_CANNOT_MOVE;
		}
	}
}

STATIC_DECLAR struct StatDebuffMsgBuf *GetExistingStatDebuffMsgBuf(struct Unit *unit)
{
	int i;
	u32 *bitfile = GetUnitStatDebuffStatus(unit)->st.bitmask;

	for (i = STAT_DEBUFF_MSG_BUF_AMT - 1; i >= 0; i--) {
		struct StatDebuffMsgBuf *buf = &sStatDebuffMsgBuf[i];

		if (buf->uid == unit->index &&
			buf->bitfile[0] == bitfile[0] &&
			buf->bitfile[1] == bitfile[1] &&
			buf->bitfile[2] == bitfile[2] &&
			buf->bitfile[3] == bitfile[3]) {
			if (i <= 3) {
				/* Generic list */
				sStatDebuffMsgBufNext = STAT_DEBUFF_MSG_BUF_NEXT(i);
			}
			return &sStatDebuffMsgBuf[i];
		}
	}
	return NULL;
}

struct StatDebuffMsgBuf *GetStatDebuffMsgBuf(struct Unit *unit)
{
	struct StatDebuffMsgBuf *buf = GetExistingStatDebuffMsgBuf(unit);

	if (!buf) {
		if (unit->index == gActiveUnit->index)
			buf = &sStatDebuffMsgBuf[STAT_DEBUFF_MSG_BUF_AMT - 3];
		else if (unit->index == gBattleActor.unit.index)
			buf = &sStatDebuffMsgBuf[STAT_DEBUFF_MSG_BUF_AMT - 2];
		else if (unit->index == gBattleTarget.unit.index)
			buf = &sStatDebuffMsgBuf[STAT_DEBUFF_MSG_BUF_AMT - 1];
		else {
			buf = &sStatDebuffMsgBuf[sStatDebuffMsgBufNext];
			sStatDebuffMsgBufNext = STAT_DEBUFF_MSG_BUF_NEXT(sStatDebuffMsgBufNext);
		}
		GenerateStatDebuffMsgBufExt(unit, GetUnitStatDebuffStatus(unit)->st.bitmask, buf);
	}

	LTRACEF("unit %#x at buf %d: pow=%d, mag=%d, skl=%d, spd=%d, lck=%d, def=%d, res=%d, mov=%d",
			UNIT_CHAR_ID(unit), buf - sStatDebuffMsgBuf,
			buf->pow, buf->mag, buf->skl, buf->spd, buf->lck, buf->def, buf->res, buf->mov);

	return buf;
}

int PowGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_HyperCutter) && (COMMON_SKILL_VALID(SID_HyperCutter)))
    if (SkillTester(unit, SID_HyperCutter))
        if (GetStatDebuffMsgBuf(unit)->pow < 0)
            return status;
#endif

#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->pow < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->pow < 0)
            return status - GetStatDebuffMsgBuf(unit)->pow;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->pow * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->pow;
}

int MagGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->mag < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->mag < 0)
            return status - GetStatDebuffMsgBuf(unit)->mag;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->mag * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->mag;
}

int SklGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->skl < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->skl < 0)
            return status - GetStatDebuffMsgBuf(unit)->skl;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->skl * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->skl;
}

int SpdGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->spd < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->spd < 0)
            return status - GetStatDebuffMsgBuf(unit)->spd;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->spd * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->spd;
}

int DefGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->def < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->def < 0)
            return status - GetStatDebuffMsgBuf(unit)->def;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->def * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->def;
}

int ResGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->res < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->res < 0)
            return status - GetStatDebuffMsgBuf(unit)->res;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->res * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->res;
}

int LckGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->lck < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->lck < 0)
            return status - GetStatDebuffMsgBuf(unit)->lck;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->lck * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->lck;
}

int MovGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->mov < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->mov < 0)
            return status - GetStatDebuffMsgBuf(unit)->mov;
#endif

#if defined(SID_ArenaTrap) && (COMMON_SKILL_VALID(SID_ArenaTrap))
    int i;
    FORCE_DECLARE bool arena_trap = false;

    for (i = 0; i < ARRAY_COUNT_RANGE2x2; i++)
    {
        int _x = unit->xPos + gVecs_2x2[i].x;
        int _y = unit->yPos + gVecs_2x2[i].y;

        struct Unit *unit_enemy = GetUnitAtPosition(_x, _y);

        if (!UNIT_IS_VALID(unit_enemy))
            continue;

        if (unit_enemy->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
            continue;

        if (AreUnitsAllied(unit->index, unit_enemy->index))
            continue;

        if (SkillTester(unit_enemy, SID_ArenaTrap))
            arena_trap = true;

        if (!arena_trap)
            continue;

        return 0;
    }
#endif
    return status + GetStatDebuffMsgBuf(unit)->mov;
}

int HpGetterStatDebuff(int status, struct Unit *unit)
{
#if (defined(SID_ClearBody) && (COMMON_SKILL_VALID(SID_ClearBody)))
    if (SkillTester(unit, SID_ClearBody))
        if (GetStatDebuffMsgBuf(unit)->maxHP < 0)
            return status;
#endif

#if (defined(SID_Contrary) && (COMMON_SKILL_VALID(SID_Contrary)))
    if (SkillTester(unit, SID_Contrary))
        if (GetStatDebuffMsgBuf(unit)->maxHP < 0)
            return status - GetStatDebuffMsgBuf(unit)->maxHP;
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        return (status + (GetStatDebuffMsgBuf(unit)->maxHP * 2));
#endif

    return status + GetStatDebuffMsgBuf(unit)->maxHP;
}

void ResetStatDeuffBuf(void)
{
	CpuFastFill16(0, sStatDebuffStatusAlly, sizeof(sStatDebuffStatusAlly));
	CpuFastFill16(0, sStatDebuffStatusEnemy, sizeof(sStatDebuffStatusEnemy));
	memset(sStatDebuffStatusNpc, 0, sizeof(sStatDebuffStatusNpc));

#ifdef CONFIG_FOURTH_ALLEGIANCE
    memset(sStatDebuffStatusFourth, 0, sizeof(sStatDebuffStatusFourth));
#endif

	memset(sStatDebuffMsgBuf, 0, sizeof(sStatDebuffMsgBuf));
	sStatDebuffMsgBufNext = 0;
}

void StatDeuff_OnNewGameInit(void)
{
	Assert(UNIT_STAT_DEBUFF_MAX == 128);
	Assert(UNIT_STAT_DEBUFF_MAX_REAL < 128);

	ResetStatDeuffBuf();
}

void StatDeuff_OnClearUnit(struct Unit *unit)
{
	memset(GetUnitStatDebuffStatus(unit)->st.bitmask, 0, sizeof(struct StatDebuffStatus));
}

void StatDeuff_OnLoadUnit(struct Unit *unit)
{
	StatDeuff_OnClearUnit(unit);
}

void StatDeuff_OnCopyUnit(struct Unit *from, struct Unit *to)
{
	memcpy(
		GetUnitStatDebuffStatus(to)->st.bitmask,
		GetUnitStatDebuffStatus(from)->st.bitmask,
		sizeof(struct StatDebuffStatus));
}

void StatDebuff_OnUnitToBattle(struct Unit *unit, struct BattleUnit *bu)
{
	*GetUnitStatDebuffStatus(&bu->unit) = *GetUnitStatDebuffStatus(unit);
}
