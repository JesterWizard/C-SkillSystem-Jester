#include "common-chax.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "debug-kit.h"
#include "kernel-lib.h"
#include "utf8.h"
#include "constants/gfx.h"

extern unsigned char const *const gpGFX_SkillIcon_WIP;

const u8 *_GetSkillIconExt(const u8 lo, int hi)
{
	const u8 *icon = gpSkillInfos[lo + (hi << 8)].icon;

	if (!icon)
		icon = gpGFX_SkillIcon_WIP;

	return icon;
}

const u8 *GetSkillIcon1(const u8 sid) { return _GetSkillIconExt(sid, SKILL_INFO_EQUIP); }
const u8 *GetSkillIcon2(const u8 sid) { return _GetSkillIconExt(sid, SKILL_INFO_CHARACTER); }
const u8 *GetSkillIcon3(const u8 sid) { return _GetSkillIconExt(sid, SKILL_INFO_CLASS); }
const u8 *GetSkillIcon4(const u8 sid) { return _GetSkillIconExt(sid, SKILL_INFO_ITEM); }

u8 GetSkillIconPal(const u16 sid)
{
	return gpSkillInfos[sid].iconPal & 0x1;
}

u8 GetIconPal(u32 icon)
{
	switch (ICON_SHEET(icon)) {
	case ICON_SHEET_SKILL0:
	case ICON_SHEET_SKILL1:
	case ICON_SHEET_SKILL2:
	case ICON_SHEET_SKILL3:
		return GetSkillIconPal(ICON_INDEX(icon));

	default:
		return 0;
	}
}

u16 GetSkillDescMsg(const u16 sid)
{
	return gpSkillInfos[sid].desc;
}

u16 GetSkillNameMsg(const u16 sid)
{
	return gpSkillInfos[sid].name;
}

u16 GetSkillPrice(const u16 sid)
{
    return gpSkillInfos[sid].price;
}

u16 GetSkillCapacity(const u16 sid)
{
    return gpSkillInfos[sid].capacity;
}


char *GetSkillDescStr(const u16 sid)
{
	char *str = NULL;
	u16 msg = GetSkillDescMsg(sid);

	if (msg != 0)
		str = GetStringFromIndex(msg);

	if (gpKernelDesignerConfig->auto_narrow_font)
		return Utf8ToNarrowFonts(str);
	else
		return str;
}

char *SkillDescToName(char *str)
{
	char *it;

	for (it = str; *it; ++it) {
		if (*it == ':') {
			*it = '\0';
			break;
		}
	}

	/**
	 * As we have changed the value in the buffer,
	 * the cached index should become invalid.
	 */
	sActiveMsg = -1;

	return str;
}

char *GetSkillNameStrFormDesc(const u16 sid)
{
	char *str;
	u16 msg = GetSkillDescMsg(sid);

	if (msg == 0)
		return "-";

	str = GetStringFromIndex(msg);
	return SkillDescToName(str);
}

char *GetSkillNameStr(const u16 sid)
{

	char *str;
	u16 msg = GetSkillNameMsg(sid);

	if (msg == 0)
		str = GetSkillNameStrFormDesc(sid);
	else
		str = GetStringFromIndex(msg);

	if (gpKernelDesignerConfig->auto_narrow_font)
		return Utf8ToNarrowFonts(str);
	else
		return str;
}

char *GetSkillNameStr_NormalFont(const u16 sid)
{
	char *str;
	u16 msg = GetSkillNameMsg(sid);

	if (msg == 0)
		str = GetSkillNameStrFormDesc(sid);
	else
		str = GetStringFromIndex(msg);
		
	return str;
}

#if 0
int GetEfxSkillPriority(const u16 sid)
{
	return gpEfxSkillAnimPriority[sid];
}
#endif

const struct EfxAnimConf *GetEfxSkillConf(const u8 aid)
{
	struct EfxAnimConf const *conf = gpEfxSkillAnims[aid];

	if (!conf)
		conf = gpEfxSkillVanilla;

	return conf;
}
