#include "gbafe.h"

/* Animations */
extern const u8 Img_EventSpriteAnim_SpawnAssassin_Blue[];
extern const u8 Img_EventSpriteAnim_SpawnRogue_Blue[];
extern const u8 Img_EventSpriteAnim_SpawnThief_Blue[];
extern const u8 ApConf_EventSpriteAnim_Spawn[];

// pal_bank = 0xA is blue, 0xB is red

static const struct EventSpriteAnimConf EventSpriteAnimConf_SpawnAssassin = {
    .pal = 0,
    .img = Img_EventSpriteAnim_SpawnAssassin_Blue,
    .ap_conf = ApConf_EventSpriteAnim_Spawn,
    .oam0 = 0x0000,
    .oam2 = 0x0200,
    .pal_bank = 0xA, // Blue palette
    .pal_size = 1
};

static const struct EventSpriteAnimConf EventSpriteAnimConf_SpawnRogue = {
    .pal = 0,
    .img = Img_EventSpriteAnim_SpawnRogue_Blue,
    .ap_conf = ApConf_EventSpriteAnim_Spawn,
    .oam0 = 0x0000,
    .oam2 = 0x0200,
    .pal_bank = 0xA, // Blue palette
    .pal_size = 1
};

static const struct EventSpriteAnimConf EventSpriteAnimConf_SpawnThief = {
    .pal = 0,
    .img = Img_EventSpriteAnim_SpawnThief_Blue,
    .ap_conf = ApConf_EventSpriteAnim_Spawn,
    .oam0 = 0x0000,
    .oam2 = 0x0200,
    .pal_bank = 0xA, // Blue palette
    .pal_size = 1
};

const struct EventSpriteAnimConf* const SpawnAnimByClass[] = {
    [CLASS_ASSASSIN] = &EventSpriteAnimConf_SpawnAssassin,
    [CLASS_ROGUE]    = &EventSpriteAnimConf_SpawnRogue,
    [CLASS_THIEF]    = &EventSpriteAnimConf_SpawnThief,
};

void StartSpawnClassFx(ProcPtr parent, int x, int y, int classId)
{
    struct ProcEventSpriteAnim* procfx;
    const struct EventSpriteAnimConf* conf;

    if (classId < 0 || classId >= (int)ARRAY_COUNT(SpawnAnimByClass))
        return;

    conf = SpawnAnimByClass[classId];
    if (!conf)
        return;

    if (parent)
        procfx = Proc_StartBlocking(ProcScr_EventSpriteAnim, parent);
    else
        procfx = Proc_Start(ProcScr_EventSpriteAnim, PROC_TREE_3);

    procfx->x = x;
    procfx->y = y;
    procfx->priv = conf;
}