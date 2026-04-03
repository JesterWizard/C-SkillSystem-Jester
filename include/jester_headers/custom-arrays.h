#include "common-chax.h"

extern const int statScreenDenyClasses[2];
extern const struct ProcCmd ProcScr_ManimLevelUp_UnitComment[];
extern const struct ClassData gClassData_NEW[];
extern const struct CharacterData gCharacterData_NEW[];
extern const struct SoundRoomEnt gSoundRoomTable_NEW[];
extern const struct SupportTalkEnt gSupportTalkList_NEW[];

struct MpSystemPInfoConfig 
{ 
    u8 initialMP;
    u8 idleGeneration; 
    u8 battleGeneration;
    u8 killGeneration;
    u8 maxMP;
};

extern const struct MpSystemPInfoConfig gMpSystemPInfoConfigList[];

struct SkillPointsSystemPInfoConfig 
{ 
    u8 standardRate;
    u8 boostedRate; 
};

extern const struct SkillPointsSystemPInfoConfig gSkillPointsSystemPInfoConfigList[];

#define MAX_BIORHYTHM_STATES 5

struct BiorhythmPInfoConfig {
    int biorhythm[MAX_BIORHYTHM_STATES];
    int startOffset; // shift starting point on turn 1
};

extern const struct BiorhythmPInfoConfig gBiorhythmPInfoConfigList[];

int GetBiorhythmBonus(struct BattleUnit* bu, int turnCounter);

#ifndef MAX_SKILL_NUM
    #define MAX_SKILL_NUM 0x3FF
#endif

extern const u16 gSkillUpgradePlusLookup[MAX_SKILL_NUM + 1]; 
extern const u16 gSkillUpgradeBaseLookup[MAX_SKILL_NUM + 1]; 

extern const u8 classIndexes_SP[6];
extern const u8 classPromotedIndexes_SP[6];
extern const u8 classWeapons_SP[6];
extern const u8 classPromotedWeapons_SP[6];

#define MAX_UNDEPLOYED_UNIT_COUNT 32
extern u8 gUndeployedUnitCount[MAX_UNDEPLOYED_UNIT_COUNT];
void SaveSuspendUnitList(u8 *dst, const u32 size);
void LoadSuspendUnitList(u8 *dst, const u32 size);

extern u8 gDeadUnits[50];
int GetDeadUnitCount(void);
void AddDeadUnit(u8 unitId);
void RemoveDeadUnit(u8 unitId);
u8 GetLastDeadUnit(void);
void SaveDeadUnits(u8 *dst, const u32 size);
void LoadDeadUnits(u8 *src, const u32 size);

extern u16 gPalEfxHpBarRed[];
extern u16 gPalEfxHpBarYellow[];
extern u16 gPalEfxHpBarBlue[];

extern const struct ProcCmd ProcScr_PhoenixStaff[];
extern const struct ProcCmd ProcScr_PhoenixRevive[];
extern const struct PopupInstruction PhoenixStaffRevivedPopup[];