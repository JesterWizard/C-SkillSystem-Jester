#pragma once
#include "gbafe.h"

struct FMUProc {
    PROC_HEADER;
    u8 uTimer;
    u16 Free;
    s8 xCur, xTo;
    s8 yCur, yTo;
    struct Unit *FMUnit;
};

typedef struct FMUProc FMUProc;
typedef bool (*ButtonFunc) (struct FMUProc*);


struct FMUTrapDef{
	u8 TrapID;
	ButtonFunc Func;
};
extern struct FMUTrapDef HookListFMU_TrapList_OnPressA[];
extern struct FMUTrapDef HookListFMU_TrapList_Auto[];


struct LocEventDef {
	u8 LocID;
	u8 TrapID;
};


extern struct LocEventDef HookListFMU_LocationBasedEvent[];
extern struct LocEventDef HookListFMU_LocationBasedEventDoor[];
extern ButtonFunc FMU_FunctionList_OnPressA[];
extern ButtonFunc FMU_FunctionList_OnPressB[];
extern ButtonFunc FMU_FunctionList_OnPressR[];
extern ButtonFunc FMU_FunctionList_OnPressL[];
extern ButtonFunc FMU_FunctionList_OnPressSelect[];
extern ButtonFunc FMU_FunctionList_OnPressStart[];

extern const u8 TimerDelay;
//#define FreeMoveFlag iFRAM[0]
extern u8* const FreeMoveFlag;

#define RunCharacterEvents ( (void(*)(u8,u8))(0x8083FB1) )
#define CheckForCharacterEvents ( (u8(*)(u8,u8))(0x8083F69) )
extern const struct ProcCmd FreeMovementControlProc[];
extern const struct MenuDef FreeMovementLMenu;
extern bool RunMiscBasedEvents(u8,u8);


/*------------- External --------------*/
bool FMU_CanUnitBeOnPos(struct Unit * unit, s8 x, s8 y);
void EnableFreeMovementASMC(void);
void DisableFreeMovementASMC(void);
u8 GetFreeMovementState(void);
void End6CInternal_FreeMU(FMUProc* proc);
void ChangeControlledUnitASMC(struct FMUProc*);
void NewPlayerPhaseEvaluationFunc(ProcPtr* ParentProc);
void NewMakePhaseControllerFunc(ProcPtr* ParentProc);
void pFMU_OnInit(struct FMUProc*);
void pFMU_InitTimer(struct FMUProc*);
void pFMU_CorrectCameraPosition(struct FMUProc*);
u8 FMU_ChkKeyForMUExtra(void);


/*------------- Core --------------*/
void pFMU_MainLoop(struct FMUProc*);
void pFMU_HanleContinueMove(struct FMUProc*);
void pFMU_MoveUnit(struct FMUProc*);
void pFMU_HandleKeyMisc(struct FMUProc*);
void pFMU_HandleSave(struct FMUProc*);
void pFMU_PressA(struct FMUProc*);
void pFMU_PressB(struct FMUProc*);
void pFMU_PressL(struct FMUProc*);
void pFMU_PressR(struct FMUProc*);
void pFMU_PressSelect(struct FMUProc*);
void pFMU_PressStart(struct FMUProc*);


/*------------- Events --------------*/
void pFMU_RunMiscBasedEvents(struct FMUProc*);
void pFMU_RunLocBasedAsmcAuto(struct FMUProc*);
bool FMUmisc_RunMapEvents(struct FMUProc*);
bool FMUmisc_RunTalkEvents(struct FMUProc*);
bool FMU_RunTrapASMC(FMUProc*);
bool FMU_RunTrapASMC_Auto(FMUProc*);

/*------------- KeyPress --------------*/
bool FMU_OnButton_StartMenu(FMUProc*);
bool FMU_OnButton_EndFreeMove(FMUProc*);
bool FMU_OnButton_ChangeUnit(FMUProc*);

