#include "common-chax.h"
#include "debuff.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "skill-system.h"
#include "constants/skills.h"

bool PostAction_BattleActorHeal(ProcPtr parent)
{
	int heal = 0;

	int hp_cur = GetUnitCurrentHp(gActiveUnit);
	int hp_max = GetUnitMaxHp(gActiveUnit);
    int missingHP = hp_max - hp_cur;

	if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
		return false;

#if defined(SID_Lifetaker) && (COMMON_SKILL_VALID(SID_Lifetaker))
	if (SkillTester(gActiveUnit, SID_Lifetaker) && gBattleActorGlobalFlag.enemy_defeated)
		heal += perc_of(hp_max, SKILL_EFF0(SID_Lifetaker));
#endif

#if defined(SID_MysticBoost) && (COMMON_SKILL_VALID(SID_MysticBoost))
	if (SkillTester(gActiveUnit, SID_MysticBoost))
		heal += SKILL_EFF0(SID_MysticBoost);
#endif

#if defined(SID_BoundlessVitality) && (COMMON_SKILL_VALID(SID_BoundlessVitality))
	if (SkillTester(gActiveUnit, SID_BoundlessVitality))
		heal += perc_of(hp_max, SKILL_EFF0(SID_BoundlessVitality));
#endif

#if defined(SID_DarkBargain) && (COMMON_SKILL_VALID(SID_DarkBargain))
    if (SkillTester(gActiveUnit, SID_DarkBargain))
    {
        if (gActiveUnit->exp >= missingHP)
        {
            gActiveUnit->exp -= missingHP;
            heal += missingHP;
        }
        else
        {
            heal += gActiveUnit->exp;
            gActiveUnit->exp = 0;
        }
    }
#endif

#if defined(SID_HoardersBane) && (COMMON_SKILL_VALID(SID_HoardersBane))
    if (SkillTester(gActiveUnit, SID_HoardersBane))
    {
        if (UNIT_CATTRIBUTES(gActiveUnit) & CA_SUPPLY || SkillTester(gActiveUnit, SID_HoardersBane))
        {
            if (gActiveUnit->curHP < gActiveUnit->maxHP)
            {
                if (GetConvoyItemSlot(ITEM_VULNERARY) != -1)
                {
                    u16 *convoy;
                    convoy = GetConvoyItemArray();
                    u16 usedItem = GetItemAfterUse(convoy[GetConvoyItemSlot(ITEM_VULNERARY)]);
                    heal += SKILL_EFF0(SID_HoardersBane);
                    RemoveItemFromConvoy(GetConvoyItemSlot(ITEM_VULNERARY));

                    if (ITEM_USES(usedItem) != 0)
                        AddItemToConvoy(usedItem);
                }
            }
        }
    }
#endif

#if defined(SID_Lifefont) && (COMMON_SKILL_VALID(SID_Lifefont))
    if (SkillTester(gActiveUnit, SID_Lifefont))
    {
        if (gActiveUnit->curHP == gActiveUnit->maxHP)
        {
            switch (gBmMapTerrain[gActiveUnit->yPos][gActiveUnit->xPos]) {
                case TERRAIN_FORT:
                case TERRAIN_THRONE:
                    heal += (GetTerrainHealAmount(gBmMapTerrain[gActiveUnit->yPos][gActiveUnit->xPos]) * gActiveUnit->maxHP / 100);
                    break;
        
                default:
                    break;
            }
        }
    }
#endif

    if (gpKernelDesignerConfig->restore_hp_on_level_up == true) 
	{
    	if (gEventSlots[EVT_SLOT_7] == 410) /* 'Heal' expressed as a hexidecimal and then convert back into decimal and summed */
		{
			if (gPlaySt.faction == FACTION_BLUE)
			{
				heal = gActiveUnit->maxHP - gActiveUnit->curHP;
			}
			else
			{
				struct Unit * targetUnit = GetUnit(gActionData.targetIndex);
				heal = targetUnit->maxHP - targetUnit->curHP;
				CallMapAnim_Heal(parent, targetUnit, heal);
				return true;
			}
		}
	}

	if ((heal >= missingHP))
		heal = missingHP;

	if (heal <= 0)
		return false;

	/**
	 * Try skip anim
	 */
	if (CheckKernelHookSkippingFlag()) {
		AddUnitHp(gActiveUnit, heal);
		return false;
	}

	CallMapAnim_Heal(parent, gActiveUnit, heal);
	return true;
}

bool PostAction_BattleTargetHeal(ProcPtr parent)
{
	struct Unit *target;
	int hp_cur, hp_max, heal;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		break;

	default:
		return false;
	}

	heal = 0;
	target = GetUnit(gActionData.targetIndex);
	hp_cur = GetUnitCurrentHp(target);
	hp_max = GetUnitMaxHp(target);

	if (!UnitAvaliable(target) || UNIT_STONED(target))
		return false;

#if defined(SID_MysticBoost) && (COMMON_SKILL_VALID(SID_MysticBoost))
	if (SkillTester(target, SID_MysticBoost))
		heal += SKILL_EFF0(SID_MysticBoost);
#endif

#if defined(SID_BoundlessVitality) && (COMMON_SKILL_VALID(SID_BoundlessVitality))
	if (SkillTester(target, SID_BoundlessVitality))
		heal += perc_of(hp_max, SKILL_EFF0(SID_BoundlessVitality));
#endif

	if (heal == 0)
		return false;

	if ((heal >= (hp_max - hp_cur)))
		heal = hp_max - hp_cur;

	/**
	 * Try skip anim
	 */
	if (CheckKernelHookSkippingFlag()) {
		AddUnitHp(target, heal);
		return false;
	}

	CallMapAnim_Heal(parent, target, heal);
	return true;
}
