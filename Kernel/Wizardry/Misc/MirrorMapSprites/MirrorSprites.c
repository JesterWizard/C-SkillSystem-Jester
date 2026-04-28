#include "gbafe.h"
#include "kernel-lib.h"
#include "common-chax.h"
#include "scene.h"

extern int gSMSSyncFlag;
extern UnitIconWait unit_icon_wait_table[];
extern struct SMSHandle gSMSHandleArray[100];
extern struct SMSHandle* gSMSHandleIt;

extern u32 gMirrorSpriteOptions;
extern u16 Pal_Grass_Tile[];
extern u16 Pal_Boulder_Tile[];
extern u16 Pal_Spin_Tile[];

#define GRASS_TRAP_OBJ_PAL 9
#define BOULDER_TRAP_OBJ_PAL 10
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

static u16 ApplyTrapSpritePalette(u16 oam2Base, const struct Trap *trap)
{
    switch (GetTrapMapSpritePalette(trap)) {
    case TRAP_MAPSPRITE_PAL_LIGHT_RUNE:
        return (oam2Base & 0x0FFF) | 0xB000;

    case TRAP_MAPSPRITE_PAL_PLAYER:
        return (oam2Base & 0x0FFF) | 0xC000;

    case TRAP_MAPSPRITE_PAL_ENEMY:
        return (oam2Base & 0x0FFF) | 0xD000;

    case TRAP_MAPSPRITE_PAL_NPC:
        return (oam2Base & 0x0FFF) | 0xE000;

    case TRAP_MAPSPRITE_PAL_GREY:
        return (oam2Base & 0x0FFF) | 0xF000;

    default:
        return oam2Base;
    }
}

static u16 ApplyGrassTrapSpritePalette(u16 oam2Base)
{
    return (oam2Base & 0x0FFF) | (GRASS_TRAP_OBJ_PAL << 12);
}

static u16 ApplyBoulderTrapSpritePalette(u16 oam2Base)
{
    return (oam2Base & 0x0FFF) | (BOULDER_TRAP_OBJ_PAL << 12);
}

static u16 ApplySpinTrapSpritePalette(u16 oam2Base)
{
    return (oam2Base & 0x0FFF) | (SPIN_TRAP_OBJ_PAL << 12);
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

static void ReloadCustomTrapSpritePalettes(void)
{
    // Do not reload these palettes if we have characters talking, as they need those palette banks as well
    if (IsTalkActive())
        return;

    ApplyPalette(Pal_Spin_Tile, 0x10 + SPIN_TRAP_OBJ_PAL);
    ApplyPalette(Pal_Grass_Tile, 0x10 + GRASS_TRAP_OBJ_PAL);
    ApplyPalette(Pal_Boulder_Tile, 0x10 + BOULDER_TRAP_OBJ_PAL);
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
    for (i = 1; i < 0xD0; i++)
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
            oam2 = UseUnitSprite(0x66) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplyTrapSpritePalette(oam2, trap);

            smsHandle->config = GetInfo(0x66).size;
        }

        if (trap->type == TRAP_HEAL_TILE)
        {
            oam2 = UseUnitSprite(0x68) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplyTrapSpritePalette(oam2, trap);

            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_TOGGLE_TORCH)
        {
            int spriteId = (trap->extra > 0) ? 0x6A : 0x6B;

            oam2 = UseUnitSprite(spriteId) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplyTrapSpritePalette(oam2, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_TELEPORT_TILE)
        {
            oam2 = UseUnitSprite(0x6C) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplyTrapSpritePalette(oam2, trap);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_GRASS_TILE)
        {
            oam2 = UseUnitSprite(0x6D) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplyGrassTrapSpritePalette(oam2);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_BOULDER_TILE)
        {
            oam2 = UseUnitSprite(0x6E) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplyBoulderTrapSpritePalette(oam2);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }

        if (trap->type == TRAP_SPIN_TILE)
        {
            oam2 = UseUnitSprite(GetSpinTrapSpriteId(trap)) - 0x5000 + 0x80;

            smsHandle = AddTrapSprite(trap->xPos * 16, trap->yPos * 16);

            smsHandle->oam2Base = ApplySpinTrapSpritePalette(oam2);
            smsHandle->config = UNIT_ICON_SIZE_16x16;
        }
    }

    if (gSMSSyncFlag != 0)
        ForceSyncUnitSpriteSheet();
}

static void PutFogStage2Sprites(void);

LYN_REPLACE_CHECK(PutUnitSpritesOam);
void PutUnitSpritesOam(void)
{
    struct SMSHandle * it = gSMSHandleArray->pNext;

    ReloadCustomTrapSpritePalettes();

    PutUnitSpriteIconsOam();

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

        if (it->config & 0x80)
            continue;

        if (it->config & 0x40)
            r3 = GetGameClock() & 2;

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

        u16 pal    = (u16)((GetUnitDisplayedSpritePalette(unit) & 0xf) << 12);
        CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200),     OAM0_Y(0x100 + y),     gObject_16x8, pal | OAM2_LAYER(2) | FOG2_CHR_TOP);
        CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200),     OAM0_Y(0x100 + y + 8), gObject_16x8, pal | OAM2_LAYER(2) | FOG2_CHR_BOTTOM);
    }
}