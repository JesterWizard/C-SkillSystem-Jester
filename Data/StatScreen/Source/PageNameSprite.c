#include "common-chax.h"

STATIC_DECLAR const u16 Sprite_Page0NameRework[] = {
	3,
	0x4104, 0x9008, TILEREF(0, 0),
	0x4104, 0x9028, TILEREF(4, 0),
};

STATIC_DECLAR const u16 Sprite_Page1NameRework[] = {
	2,
	0x4104, 0x901E, TILEREF(0, 0),
	0x4104, 0x903E, TILEREF(4, 0),
};

STATIC_DECLAR const u16 Sprite_Page2NameRework[] = {
	4,
	0x4104, 0x9008, TILEREF(0, 0),
	0x4104, 0x9028, TILEREF(4, 0),
	0x4104, 0x9048, TILEREF(8, 0),
};

STATIC_DECLAR const u16 Sprite_Page3NameRework[] = {
	3,
	0x4104, 0x9008, TILEREF(0, 0),
	0x4104, 0x9028, TILEREF(4, 0),
	0x4104, 0x9048, TILEREF(8, 0),
};

STATIC_DECLAR const u16 Sprite_Page4NameRework[] = {
	3,
	0x4104, 0x9008, TILEREF(0, 0),
	0x4104, 0x9028, TILEREF(4, 0),
	0x4104, 0x9040, TILEREF(7, 0),
};

STATIC_DECLAR const u16 Sprite_Page5NameRework[] =
{
    3,
    0x4104, 0x9008, TILEREF(0, 0),
    0x4104, 0x9028, TILEREF(4, 0),
    0x4104, 0x9048, TILEREF(8, 0),
};


STATIC_DECLAR const u16 Sprite_Page6NameRework[] =
{
    3,
    0x4104, 0x9008, TILEREF(0, 0),
    0x4104, 0x9028, TILEREF(4, 0),
    0x4104, 0x9038, TILEREF(6, 0),
};

u16 const *const Sprites_PageNameRework[] = {
		Sprite_Page0NameRework,
		Sprite_Page1NameRework,
		Sprite_Page2NameRework,
		Sprite_Page3NameRework,
#ifdef CONFIG_MP_SYSTEM
		Sprite_Page4NameRework,
#endif
#ifdef CONFIG_STAT_PAGE_PERSONAL_INFO
		Sprite_Page5NameRework,
#endif
#ifdef CONFIG_STAT_PAGE_PROMOTIONS
		Sprite_Page6NameRework,
#endif
};

/*
** This is the offset each new page should point to, starting at page 1 (0x240)
** E.g. Page 3 will point at 0x240 + 0xE in the OAM tile space (in NoCashGBA for reference)
** At least that's how it should work in theory, in practice from page 5 onwards it's a free for all, I dunno -\/(:)\/-
*/

const u16 gPageNameChrOffsetLutRe[] = {
	0x18, // Page 1 etc
	0x40, 
	0x0C, 
	0x80, 
	0x95, 
	0x00, 
	0x8B
};