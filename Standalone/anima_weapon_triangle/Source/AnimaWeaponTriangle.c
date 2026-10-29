#include "global.h"
#include "bmitem.h"
#include "bmbattle.h"

struct WeaponTriangleRule {
	s8 attackerWeaponType;
	s8 defenderWeaponType;
	s8 hitBonus;
	s8 atkBonus;
};

/*
 * One row per tome: the tome and the two tomes it beats.
 * The list ends with a row whose first byte is 0.
 * Defined in Installer.event (edit there, no recompile).
 */
struct AnimaTriangleEntry {
	u8 item;
	u8 beats[2];
};

extern const struct AnimaTriangleEntry AnimaTriangleTable[];
extern const s8 AnimaTriangleAtkBonus;
extern const s8 AnimaTriangleHitBonus;

/*
 * BattleApplyWeaponTriangleEffect reads its rule table through the literal at
 * 0x0802C7CC. The hook only overwrites the first 8 bytes of the function, so
 * that literal is still there and still points at the table FEBuilder edits.
 */
#define VanillaWeaponTriangleRules (*(const struct WeaponTriangleRule * const *)0x0802C7CC)


static bool AnimaBeats(int item, int other)
{
	const struct AnimaTriangleEntry *it;

	if (item == 0 || other == 0)
		return false;

	for (it = AnimaTriangleTable; it->item != 0; it++) {
		if (it->item == item)
			return it->beats[0] == other || it->beats[1] == other;
	}

	return false;
}

static void ApplyAnimaTriangle(struct BattleUnit *winner, struct BattleUnit *loser)
{
	winner->wTriangleHitBonus += AnimaTriangleHitBonus;
	winner->wTriangleDmgBonus += AnimaTriangleAtkBonus;

	loser->wTriangleHitBonus -= AnimaTriangleHitBonus;
	loser->wTriangleDmgBonus -= AnimaTriangleAtkBonus;
}

/**
 * Vanilla BattleApplyWeaponTriangleEffect (weapon-type triangle and reaver
 * reversal), then the C Skill System anima_weapon_triangle rule: when one
 * unit's tome beats the other's (Fire > Wind/Ice > Thunder > Fire by
 * default), the winner gets +atk/+hit and the loser gets -atk/-hit.
 */
void BattleApplyWeaponTriangleEffect_Anima(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	const struct WeaponTriangleRule *it;
	int a, d;

	for (it = VanillaWeaponTriangleRules; it->attackerWeaponType >= 0; ++it) {
		if ((attacker->weaponType == it->attackerWeaponType) && (defender->weaponType == it->defenderWeaponType)) {
			attacker->wTriangleHitBonus = it->hitBonus;
			attacker->wTriangleDmgBonus = it->atkBonus;

			defender->wTriangleHitBonus = -it->hitBonus;
			defender->wTriangleDmgBonus = -it->atkBonus;

			break;
		}
	}

	if (attacker->weaponAttributes & IA_REVERTTRIANGLE)
		BattleApplyReaverEffect(attacker, defender);

	if (defender->weaponAttributes & IA_REVERTTRIANGLE)
		BattleApplyReaverEffect(attacker, defender);

	a = ITEM_INDEX(attacker->weaponBefore);
	d = ITEM_INDEX(defender->weaponBefore);

	if (AnimaBeats(a, d))
		ApplyAnimaTriangle(attacker, defender);
	else if (AnimaBeats(d, a))
		ApplyAnimaTriangle(defender, attacker);
}
