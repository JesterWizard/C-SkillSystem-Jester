#include "gbafe.h"

/* Animations */
extern const u8 Img_EventSpriteAnim_SpawnAssassin_Blue[];
extern const u8 Img_EventSpriteAnim_SpawnRogue_Blue[];
extern const u8 Img_EventSpriteAnim_SpawnThief_Blue[];
extern const u8 ApConf_EventSpriteAnim_Spawn[];

/*
 * pal_bank: the animation's OBJ palette, 2 under the map sprites' (OBJ 0xC blue, 0xD red, 0xE green): 0xA blue, 0xB red,
 * 0xC green. The images are one drawing; the unit's faction picks the colours (the red image's own palette is OBJ
 * 0xD's). An enemy teleported in blue before.
 */
#define SPAWN_CONF(image, bank) { \
    .pal = 0, \
    .img = image, \
    .ap_conf = ApConf_EventSpriteAnim_Spawn, \
    .oam0 = 0x0000, \
    .oam2 = 0x0200, \
    .pal_bank = bank, \
    .pal_size = 1 \
}

#define SPAWN_CONFS(image) { SPAWN_CONF(image, 0xA), SPAWN_CONF(image, 0xB), SPAWN_CONF(image, 0xC) }

static const struct EventSpriteAnimConf sSpawnAssassin[3] = SPAWN_CONFS(Img_EventSpriteAnim_SpawnAssassin_Blue);
static const struct EventSpriteAnimConf sSpawnRogue[3] = SPAWN_CONFS(Img_EventSpriteAnim_SpawnRogue_Blue);
static const struct EventSpriteAnimConf sSpawnThief[3] = SPAWN_CONFS(Img_EventSpriteAnim_SpawnThief_Blue);

/* By class, then faction (0 blue, 1 red, 2 green) */
static const struct EventSpriteAnimConf *const SpawnAnimByClass[] = {
    [CLASS_ASSASSIN] = sSpawnAssassin,
    [CLASS_ROGUE]    = sSpawnRogue,
    [CLASS_THIEF]    = sSpawnThief,
};

/* x, y: screen pixels; faction: FACTION_BLUE, FACTION_RED or FACTION_GREEN */
void StartSpawnClassFxFaction(ProcPtr parent, int x, int y, int classId, int faction)
{
    struct ProcEventSpriteAnim* procfx;
    const struct EventSpriteAnimConf* conf;
    int pal = faction == FACTION_RED ? 1 : faction == FACTION_GREEN ? 2 : 0;

    if (classId < 0 || classId >= (int)ARRAY_COUNT(SpawnAnimByClass))
        return;

    conf = SpawnAnimByClass[classId];
    if (!conf)
        return;
    conf += pal;

    if (parent)
        procfx = Proc_StartBlocking(ProcScr_EventSpriteAnim, parent);
    else
        procfx = Proc_Start(ProcScr_EventSpriteAnim, PROC_TREE_3);

    procfx->x = x;
    procfx->y = y;
    procfx->priv = conf;
}

/* The player's colours */
void StartSpawnClassFx(ProcPtr parent, int x, int y, int classId)
{
    StartSpawnClassFxFaction(parent, x, y, classId, FACTION_BLUE);
}
