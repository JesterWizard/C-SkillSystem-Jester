#include "common-chax.h"
#include "kernel-lib.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

/*
 * Hex save data: the card binder (copies owned of each card type) and the district claim counters.
 * gHexSave lives in RAM (include/link/config-memmap.s) and is saved as one chunk in each save list
 * (Kernel/Wizardry/SaveData/data.event, 0x54 bytes). HexSave_NewGame is a new-game hook.
 */

#define HEX_SAVE_MAGIC 0x4858 /* "HX" */
#define HEX_CLAIM_MAX 255

/* The layout is what the 0x54-byte chunk and the binder bounds assume */
typedef char HexSaveData_SizeCheck[(sizeof(struct HexSaveData) == 0x54) ? 1 : -1];
typedef char HexBinderSlots_Check[(HEX_CARD_TYPES <= HEX_BINDER_SLOTS) ? 1 : -1];

static bool BinderTypeInRange(int type)
{
	return type >= 0 && type < HEX_CARD_TYPES;
}

static bool ClaimSlotInRange(int slot)
{
	return slot >= 0 && slot < HEX_CLAIM_SLOTS;
}

static void HexSave_Clear(void)
{
	CpuFill16(0, &gHexSave, sizeof(gHexSave));
	gHexSave.magic = HEX_SAVE_MAGIC;
}

/* Copies owned of a card type. 0 for a bad type. */
int HexBinder_Get(int type)
{
	if (!BinderTypeInRange(type))
		return 0;

	return gHexSave.binder[type];
}

/* FALSE at HEX_BINDER_MAX or for a bad type. */
bool HexBinder_Add(int type)
{
	if (!BinderTypeInRange(type) || gHexSave.binder[type] >= HEX_BINDER_MAX)
		return false;

	gHexSave.binder[type]++;
	return true;
}

/* FALSE when none are owned or for a bad type. */
bool HexBinder_Remove(int type)
{
	if (!BinderTypeInRange(type) || gHexSave.binder[type] == 0)
		return false;

	gHexSave.binder[type]--;
	return true;
}

int HexBinder_Total(void)
{
	int total = 0;
	int i;

	for (i = 0; i < HEX_CARD_TYPES; i++)
		total += gHexSave.binder[i];

	return total;
}

/* Times the district signature card in this slot was claimed. 0 for a bad slot. */
int HexClaim_Get(int slot)
{
	if (!ClaimSlotInRange(slot))
		return 0;

	return gHexSave.claims[slot];
}

/* Saturates at HEX_CLAIM_MAX; ignores bad slots. */
void HexClaim_Add(int slot)
{
	if (!ClaimSlotInRange(slot) || gHexSave.claims[slot] >= HEX_CLAIM_MAX)
		return;

	gHexSave.claims[slot]++;
}

/* New-game hook (gNewSaveHooks) */
void HexSave_NewGame(void)
{
	HexSave_Clear();
}

/* Game and suspend save chunk (0x54 bytes) */
void HexSave_Save(u8 *dst, const u32 size)
{
	gHexSave.magic = HEX_SAVE_MAGIC;
	WriteAndVerifySramFast(&gHexSave, dst, sizeof(gHexSave));
}

/* Saves made before this chunk existed have no magic: they load an empty binder and no claims */
void HexSave_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gHexSave, sizeof(gHexSave));

	if (gHexSave.magic != HEX_SAVE_MAGIC)
		HexSave_Clear();
}

#endif /* CONFIG_HEX_CARDS */
