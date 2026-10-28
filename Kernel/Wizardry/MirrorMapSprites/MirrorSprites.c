#include "gbafe.h"
#include "kernel-lib.h"
#include "common-chax.h"
#include "kernel/chatlog.h"
#include "kernel/traps.h"

extern int gSMSSyncFlag;
extern UnitIconWait unit_icon_wait_table[];
extern struct SMSHandle gSMSHandleArray[100];
extern struct SMSHandle* gSMSHandleIt;

extern u32 gMirrorSpriteOptions;
extern int GetDangerBonesShakeOffset(int shaking);
extern u16 Pal_Grass_Tile[];
extern u16 Pal_Spin_Tile[];

#define GRASS_TRAP_OBJ_PAL 9
#define SPIN_TRAP_OBJ_PAL 8

enum {
	FLIP_PLAYER = 0x1,
	FLIP_ENEMY = 0x2,
	FLIP_NPC = 0x4,
	FLIP_FOURTH = 0x8,
};

#define GetInfo(id) (unit_icon_wait_table[(id) & ((1<<7)-1)])

const u16 gObject_16x16_HFlipped[] =
{
    1, OAM0_SHAPE_16x16, OAM1_SIZE_16x16 + OAM1_HFLIP, 0,
};


const u16 gObject_16x32_HFlipped[] =
{
    1, OAM0_SHAPE_16x32, OAM1_SIZE_16x32 + OAM1_HFLIP, 0,
};

const u16 gObject_32x32_HFlipped[] =
{
    1, OAM0_SHAPE_32x32, OAM1_SIZE_32x32 + OAM1_HFLIP, 0,
};

/* Map sprite OBJ palette banks: trap art is indexed against the shared unit palette (banks C-F) */
#define LIGHT_RUNE_OBJ_PAL 0xB
#define PLAYER_OBJ_PAL 0xC

int GetTrapMapSpriteObjPal(const struct Trap *trap)
{
    int nativePal = PLAYER_OBJ_PAL;

    switch (trap->type) {
    case TRAP_GRASS_TILE:
        return GRASS_TRAP_OBJ_PAL;

    case TRAP_SPIN_TILE:
        return SPIN_TRAP_OBJ_PAL;

    case TRAP_LIGHT_RUNE:
        nativePal = LIGHT_RUNE_OBJ_PAL;
        break;
    }

    switch (GetTrapMapSpritePalette(trap)) {
    case TRAP_MAPSPRITE_PAL_PLAYER:
        return PLAYER_OBJ_PAL;

    case TRAP_MAPSPRITE_PAL_ENEMY:
        return 0xD;

    case TRAP_MAPSPRITE_PAL_NPC:
        return 0xE;

    case TRAP_MAPSPRITE_PAL_GREY:
        return 0xF;

    /* Only the light rune's own art is indexed for the light rune bank */
    case TRAP_MAPSPRITE_PAL_LIGHT_RUNE:
    default:
        return nativePal;
    }
}

static u16 GetTrapSpriteOam2(int spriteId, const struct Trap *trap)
{
    return ((UseUnitSprite(spriteId) + 0x80) & 0x0FFF) | (GetTrapMapSpriteObjPal(trap) << 12);
}

static int GetSpinTrapSpriteId(const struct Trap *trap)
{
    if (!trap)
        return 0x6F;

    switch (trap->data[TRAP_EXTDATA_SPIN_TILE_DIRECTION]) {
    case SPIN_TILE_DIR_LEFT:
        return 0x70;

    case SPIN_TILE_DIR_UP:
        return 0x71;

    case SPIN_TILE_DIR_DOWN:
        return 0x72;

    case SPIN_TILE_DIR_RIGHT:
    default:
        return 0x6F;
    }
}

static bool IsTrapSpritePaletteLoaded(const u16 *pal, int bank)
{
    const u16 *loaded = gPaletteBuffer + bank * 0x10;
    int i;

    for (i = 0; i < 0x10; i++)
        if (loaded[i] != pal[i])
            return false;

    return true;
}

static bool MapHasTrapType(int type)
{
    struct Trap *trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++)
        if (trap->type == type)
            return true;

    return false;
}

static const u16 *GetTrapCustomSpritePalette(int type, int *bank)
{
    switch (type) {
    case TRAP_GRASS_TILE:
        *bank = GRASS_TRAP_OBJ_PAL;
        return Pal_Grass_Tile;

    case TRAP_SPIN_TILE:
        *bank = SPIN_TRAP_OBJ_PAL;
        return Pal_Spin_Tile;

    default:
        return NULL;
    }
}

/* Uploads the dedicated OBJ palette bank of a trap type that has one, if it isn't already there */
void LoadTrapMapSpritePalette(int type)
{
    int bank;
    const u16 *pal = GetTrapCustomSpritePalette(type, &bank);

    if (pal && !IsTrapSpritePaletteLoaded(pal, 0x10 + bank))
        ApplyPalette(pal, 0x10 + bank);
}

static bool TrapSpritePaletteMissing(int type)
{
    int bank;
    const u16 *pal = GetTrapCustomSpritePalette(type, &bank);

    return !IsTrapSpritePaletteLoaded(pal, 0x10 + bank) && MapHasTrapType(type);
}

static void ReloadCustomTrapSpritePalettes(void)
{
    /**
     * Runs every frame. Banks 8/9 are shared with faces and map anim MUs,
     * so only claim one while a trap that draws with it is on the map.
     */
    bool grass = TrapSpritePaletteMissing(TRAP_GRASS_TILE);
    bool spin = TrapSpritePaletteMissing(TRAP_SPIN_TILE);

    if (!(grass || spin) || IsTalkActive())
        return;

    if (grass)
        LoadTrapMapSpritePalette(TRAP_GRASS_TILE);

    if (spin)
        LoadTrapMapSpritePalette(TRAP_SPIN_TILE);
}

static struct SMSHandle *AddTrapSprite(int xDisplay, int yDisplay)
{
    struct SMSHandle *smsHandle = AddUnitSprite(yDisplay);

    smsHandle->yDisplay = yDisplay;
    smsHandle->xDisplay = xDisplay;
    smsHandle->_u0A = 0;

    return smsHandle;
}

LYN_REPLACE_CHECK(RefreshUnitSprites);
void RefreshUnitSprites(void)
{
    struct SMSHandle * smsHandle;

    struct Trap * trap;
    int i;
    u16 oam2 = 0;
    struct SMSHandle * nullHandle = NULL;

    gSMSHandleIt = &gSMSHandleArray[0];

    gSMSHandleIt->pNext = nullHandle;
    gSMSHandleIt->yDisplay = 0x400;

    gSMSHandleIt = &gSMSHandleArray[1];

    ReloadCustomTrapSpritePalettes();

#ifdef CONFIG_FOURTH_ALLEGIANCE
    for (i = 1; i < (gpKernelDesignerConfig->fourth_allegiance ? 0xD0 : 0xC0); i++)
#else
    for (i = 1; i < 0xC0; i++)
#endif
    {
        struct Unit * unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        unit->pMapSpriteHandle = NULL;

        if (unit->state & (US_HIDDEN | US_BIT9))
            continue;

        /* Stage 2 fog: suppress the real SMS sprite here; the link arena hidden
         * sprite is drawn separately in PutUnitSpritesOam instead. */
        if (gpKernelDesignerConfig->multiple_fog_stages == true
                && gPlaySt.chapterVisionRange && gBmMapFog[unit->yPos][unit->xPos] == 1)
            continue;

        if (gBmMapUnit[unit->yPos][unit->xPos] == 0)
            continue;

        if (unit->statusIndex == UNIT_STATUS_PETRIFY || unit->statusIndex == UNIT_STATUS_13)
            unit->state |= US_UNSELECTABLE;

        smsHandle = AddUnitSprite(unit->yPos * 16);

        smsHandle->yDisplay = unit->yPos * 16;
        smsHandle->xDisplay = unit->xPos * 16;

        if (unit->state & US_RESCUING) {
            smsHandle->xDisplay += 2;
            smsHandle->yDisplay += 2;
        }

        smsHandle->oam2Base = UseUnitSprite(GetUnitSMSId(unit)) + 0x80 + (GetUnitDisplayedSpritePalette(unit) & 0xf) * 0x1000;

		//SMSHandle._u0A appears to be unused, so I'll use it to track who should be flipped
		smsHandle->_u0A = 0;
		switch (UNIT_FACTION(unit)) {
			case FACTION_BLUE:
				if (gMirrorSpriteOptions & FLIP_PLAYER) {
					smsHandle->_u0A = 1;
				}
				break;
			case FACTION_RED:
				if (gMirrorSpriteOptions & FLIP_ENEMY) {
					smsHandle->_u0A = 1;
				}
				break;
			case FACTION_GREEN:
				if (gMirrorSpriteOptions & FLIP_NPC) {
					smsHandle->_u0A = 1;
				}
				break;
			case FACTION_PURPLE:
				if (gMirrorSpriteOptions & FLIP_FOURTH) {
					smsHandle->_u0A = 1;
				}
				break;
		}

        smsHandle->config = GetInfo(GetUnitSMSId(unit)).size;

        if (unit->state & 0x100) {
            smsHandle->config += 3;
        }

        if (unit->state & 0x1000000) {
            smsHandle->config += 0x40;
        }

        unit->pMapSpriteHandle = smsHandle;
    }

    for (trap = GetTrap(0); trap->type != 0; trap++)
    {
        if (trap->type == 1 && trap->data[1] == 0)
        {
            switch (trap->extra) {
            case 0x35:
                oam2 = UseUnitSprite(0x5b) - 0x4000 + 0x80;
                break;

            case 0x36:
                oam2 = UseUnitSprite(0x5c) - 0x4000 + 0x80;
                break;

            case 0x37:
                oam2 = UseUnitSprite(0x5d) - 0x4000 + 0x80;
                break;
            }

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = oam2;

            smsHandle->config = GetInfo(0x5b).size;
        }

        if (trap->type == TRAP_LIGHT_RUNE)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x66, trap);

            smsHandle->config = GetInfo(0x66).size;
        }

        if (trap->type == TRAP_HEAL_TILE)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x68, trap);

            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_TOGGLE_TORCH)
        {
            int spriteId = (trap->extra > 0) ? 0x6A : 0x6B;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(spriteId, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_TELEPORT_TILE)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x6C, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_GRASS_TILE)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x6D, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_BOULDER_TILE)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x6E, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_SPIN_TILE)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(GetSpinTrapSpriteId(trap), trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_DRAGON_VEIN)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x69, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_HELLSCAPE_VOID)
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x73, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        /* Hidden caltrops are only drawn once revealed */
        if (trap->type == TRAP_HIDDEN_CALTROPS && trap->data[TRAP_EXTDATA_HIDDEN_CALTROPS_REVEALED])
        {
            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = GetTrapSpriteOam2(0x74, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }
    }

    if (gSMSSyncFlag != 0)
        ForceSyncUnitSpriteSheet();
}

static void PutFogStage2Sprites(void);

static void PutRescuedSupportSpritesOam(void)
{
	int i;

	/* Only Pair Up draws the carried unit */
	if (!gpKernelDesignerConfig->pair_up_enabled)
		return;

	for (i = 1; i <= 0xC0 + CONFIG_UNIT_AMT_FOURTH; ++i) {
		/* Same table GetUnit() reads, without a long call per slot every frame */
		struct Unit *leader = gUnitLookup[i];
		struct Unit *support;
		struct SMSHandle sms;
		int x;
		int y;
		u16 oam2;
		bool flip = false;

		if (!leader || !UNIT_IS_VALID(leader)
			|| !(leader->state & US_RESCUING))
			continue;

		support = leader->rescue ? GetUnit(leader->rescue) : NULL;
		if (!support || !UNIT_IS_VALID(support)
			|| !(support->state & US_RESCUED)
			|| !(support->state & US_HIDDEN))
			continue;

		/* Do not draw a rescued enemy as a Pair Up support. */
		if (!AreUnitsAllied(leader->index, support->index)
			|| !IsSameAllegiance(leader->index, support->index))
			continue;

		if (leader->state & (US_HIDDEN | US_BIT9))
			continue;

		x = leader->xPos * 16 - gBmSt.camera.x - 6;
		y = leader->yPos * 16 - gBmSt.camera.y - 6;

		if (x < -16 || x > DISPLAY_WIDTH || y < -32 || y > DISPLAY_HEIGHT)
			continue;

		x += TextEngine_GetStaticOffsetAtY(y);
		y += TextEngine_GetFxOffsetY();

		switch (UNIT_FACTION(support)) {
		case FACTION_BLUE:
			flip = gMirrorSpriteOptions & FLIP_PLAYER;
			break;
		case FACTION_RED:
			flip = gMirrorSpriteOptions & FLIP_ENEMY;
			break;
		case FACTION_GREEN:
			flip = gMirrorSpriteOptions & FLIP_NPC;
			break;
		case FACTION_PURPLE:
			flip = gMirrorSpriteOptions & FLIP_FOURTH;
			break;
		}

		sms.config = GetInfo(GetUnitSMSId(support)).size;
		oam2 = UseUnitSprite(GetUnitSMSId(support)) + 0x80
			+ (GetUnitDisplayedSpritePalette(leader) & 0xF) * 0x1000;

		if (flip) {
			switch (sms.config & 0xF) {
			case 0:
				CallARM_PushToSecondaryOAM(
					OAM1_X(x + 0x200), OAM0_Y(0x100 + y),
					gObject_16x16_HFlipped, oam2 + OAM2_LAYER(3));
				break;
			case 1:
				CallARM_PushToSecondaryOAM(
					OAM1_X(x + 0x200), OAM0_Y(0x100 + y - 16),
					gObject_16x32_HFlipped, oam2 + OAM2_LAYER(3));
				break;
			case 2:
				CallARM_PushToSecondaryOAM(
					OAM1_X(x - 8 + 0x200), OAM0_Y(0x100 + y - 16),
					gObject_32x32_HFlipped, oam2 + OAM2_LAYER(3));
				break;
			default:
				break;
			}
		} else {
			switch (sms.config & 0xF) {
			case 0:
				CallARM_PushToSecondaryOAM(
					OAM1_X(x + 0x200), OAM0_Y(0x100 + y),
					gObject_16x16, oam2 + OAM2_LAYER(3));
				break;
			case 1:
				CallARM_PushToSecondaryOAM(
					OAM1_X(x + 0x200), OAM0_Y(0x100 + y - 16),
					gObject_16x32, oam2 + OAM2_LAYER(3));
				break;
			case 2:
				CallARM_PushToSecondaryOAM(
					OAM1_X(x - 8 + 0x200), OAM0_Y(0x100 + y - 16),
					gObject_32x32, oam2 + OAM2_LAYER(3));
				break;
			default:
				break;
			}
		}
	}
}

LYN_REPLACE_CHECK(PutUnitSpritesOam);
void PutUnitSpritesOam(void)
{
	/* Map sprites sit above BG2; hide them while the chatlog owns the screen. */
	if (Chatlog_IsVisible())
		return;

    struct SMSHandle * it = gSMSHandleArray->pNext;

    ReloadCustomTrapSpritePalettes();

    PutUnitSpriteIconsOam();
    PutRescuedSupportSpritesOam();

    if (it == NULL)
        return;

    for (; it != NULL; it = it->pNext)
    {
        int r3 = 0;

        int x = it->xDisplay - gBmSt.camera.x;
        int y = it->yDisplay - gBmSt.camera.y;

        if (x < -16 || x > DISPLAY_WIDTH)
            continue;

        if (y < -32 || y > DISPLAY_HEIGHT)
            continue;

        x += TextEngine_GetStaticOffsetAtY(y);
        y += TextEngine_GetFxOffsetY();

        if (it->config & 0x80)
            continue;

        if (it->config & 0x40)
            r3 = GetDangerBonesShakeOffset(true);

		if (gpKernelDesignerConfig->flipped_enemy_sprites == true && it->_u0A == 1) {
			switch ((it->config & 0xf)) {
		    case 0:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y), gObject_16x16_HFlipped, it->oam2Base + OAM2_LAYER(2));
		        break;

		    case 1:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y - 16), gObject_16x32_HFlipped, it->oam2Base + OAM2_LAYER(2));
		        break;

		    case 2:
		        CallARM_PushToSecondaryOAM(OAM1_X((x-8)+r3+0x200), OAM0_Y(0x100+y - 16), gObject_32x32_HFlipped, it->oam2Base + OAM2_LAYER(2));
		        break;

		    case 3:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y), gObject_16x16_HFlipped, it->oam2Base + OAM2_LAYER(3));;
		        break;

		    case 4:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y - 16), gObject_16x32_HFlipped, it->oam2Base + OAM2_LAYER(3));
		        break;

		    case 5:
		        CallARM_PushToSecondaryOAM(OAM1_X((x-8)+r3+0x200), OAM0_Y(0x100+y - 16), gObject_32x32_HFlipped, it->oam2Base + OAM2_LAYER(3));
		        break;
		    }
		}

		else {
		    switch ((it->config & 0xf)) {
		    case 0:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y), gObject_16x16, it->oam2Base + OAM2_LAYER(2));
		        break;

		    case 1:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y - 16), gObject_16x32, it->oam2Base + OAM2_LAYER(2));
		        break;

		    case 2:
		        CallARM_PushToSecondaryOAM(OAM1_X((x-8)+r3+0x200), OAM0_Y(0x100+y - 16), gObject_32x32, it->oam2Base + OAM2_LAYER(2));
		        break;

		    case 3:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y), gObject_16x16, it->oam2Base + OAM2_LAYER(3));;
		        break;

		    case 4:
		        CallARM_PushToSecondaryOAM(OAM1_X(x+r3+0x200), OAM0_Y(0x100+y - 16), gObject_16x32, it->oam2Base + OAM2_LAYER(3));
		        break;

		    case 5:
		        CallARM_PushToSecondaryOAM(OAM1_X((x-8)+r3+0x200), OAM0_Y(0x100+y - 16), gObject_32x32, it->oam2Base + OAM2_LAYER(3));
		        break;
		    }
		}
    }

    if (gpKernelDesignerConfig->multiple_fog_stages == true)
        PutFogStage2Sprites();
}

/* Draws the link arena "hidden unit" sprite (gUnknown_085AD9CC, decompressed
 * to OBJ VRAM tile 0x1F0 by sub_8049788) for any enemy at fog stage 2
 * (gBmMapFog == 1). Mirrors the visual logic of sub_804B278 for the field map. */
static void PutFogStage2Sprites(void)
{
    if (!gPlaySt.chapterVisionRange)
        return;

    /* Load the hidden-unit sprite sheet into OBJ VRAM. Called every frame
     * so VRAM stays coherent across chapter transitions and palette changes. */
    sub_8049788();

    /* CHR tile indices (OAM2 bits 0-9) matching sub_804B278's 0x9f0 / 0x9F2.
     * Those values encode tile 0x1F0 / 0x1F2 with priority bits in bits 10-11. */
    enum {
        FOG2_CHR_TOP    = 0x1F0, /* top    16x8 half of the hidden sprite */
        FOG2_CHR_BOTTOM = 0x1F2, /* bottom 16x8 half */
    };

    /* 1-pixel bob: alternates each 8 game-clock ticks. */
    int yBob = (GetGameClock() >> 3) & 1;

    for (int i = FACTION_RED + 1; i < FACTION_PURPLE; i++) {
        struct Unit *unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;
        if (unit->state & US_HIDDEN)
            continue;
        if (gBmMapFog[unit->yPos][unit->xPos] != 1)
            continue;

        int x = unit->xPos * 16 - gBmSt.camera.x;
        int y = unit->yPos * 16 - gBmSt.camera.y - yBob;

        if (x < -16 || x > DISPLAY_WIDTH)  continue;
        if (y < -16 || y > DISPLAY_HEIGHT) continue;

        x += TextEngine_GetStaticOffsetAtY(y);
        y += TextEngine_GetFxOffsetY();

        u16 pal    = (u16)((GetUnitDisplayedSpritePalette(unit) & 0xf) << 12);
        CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200),     OAM0_Y(0x100 + y),     gObject_16x8, pal | OAM2_LAYER(2) | FOG2_CHR_TOP);
        CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200),     OAM0_Y(0x100 + y + 8), gObject_16x8, pal | OAM2_LAYER(2) | FOG2_CHR_BOTTOM);
    }
}

void MU_DisplayAsSMS(struct MuProc *proc);
void MU_DisplayAsMMS(struct MuProc *proc);

LYN_REPLACE_CHECK(MU_DisplayAsSMS);
void MU_DisplayAsSMS(struct MuProc *proc)
{
	struct Vec2 pos;

	if (proc->hidden_b)
		return;

	if (!GetMuDisplayPosition(proc, &pos))
		return;

	pos.x += TextEngine_GetStaticOffsetAtY(pos.y);
	pos.y += TextEngine_GetFxOffsetY();
	pos.x = OAM1_X(pos.x);
	pos.y = OAM0_Y(pos.y);

	if (proc->state == MU_STATE_DEATHFADE)
		pos.y |= OAM0_BLEND;

	sub_8026FF4(proc->slot, proc->pGfxVRAM);
	sub_8027DB4(
		proc->sprite_anim->objLayer,
		pos.x - 8,
		pos.y - 16,
		((((unsigned)((uintptr_t)proc->pGfxVRAM - 0x06010000) & 0x1FFFF) >> 5)
			| ((proc->config->pal & 0xF) << 12))
			+ proc->layer,
		proc->jid,
		proc->slot
	);
}

LYN_REPLACE_CHECK(MU_DisplayAsMMS);
void MU_DisplayAsMMS(struct MuProc *proc)
{
	struct Vec2 pos;

	if (proc->hidden_b)
		return;

	if (!GetMuDisplayPosition(proc, &pos))
		return;

	if (proc->state != MU_STATE_DISPLAY_UI)
		if (proc->unit && UNIT_FACTION(proc->unit) == FACTION_RED)
			if (gPlaySt.chapterVisionRange != 0)
				if (gBmMapFog[MU_GetDisplayYOrg(proc) >> 4][MU_GetDisplayXOrg(proc) >> 4] == 0)
					return;

	pos.x += TextEngine_GetStaticOffsetAtY(pos.y);
	pos.y += TextEngine_GetFxOffsetY();
	pos.x = OAM1_X(pos.x);
	pos.y = OAM0_Y(pos.y);

	if (proc->state == MU_STATE_DEATHFADE)
		pos.y |= OAM0_BLEND;

	AP_Update(proc->sprite_anim, pos.x, pos.y);
}