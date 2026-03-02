#include "FreeMU.h"
#include "common-chax.h"
#include "kernel-lib.h"

/*
 * Basic! 
 */
bool FMU_CanUnitBeOnPos(struct Unit* unit, s8 x, s8 y){
	if (x < 0 || y < 0)
		return 0; // position out of bounds
	if (x >= gBmMapSize.x || y >= gBmMapSize.y)
		return 0; // position out of bounds
	if (gBmMapUnit[y][x])
		return 0; // a unit is occupying this position
	if (gBmMapHidden[y][x] & 1)
		return 0; // a hidden unit is occupying this position	
	return CanUnitCrossTerrain(unit, gBmMapTerrain[y][x]);
}

void EnableFreeMovementASMC(void){
	if (gpKernelDesignerConfig->free_movement == true)
	{
		*FreeMoveFlag |= 1;
	}
	return;
}
 
void DisableFreeMovementASMC(void){
	*FreeMoveFlag = (*FreeMoveFlag>>1)<<1;
	return;
}

u8 GetFreeMovementState(void){
	return (*FreeMoveFlag&1);
}

void End6CInternal_FreeMU(FMUProc* proc){
	DisableFreeMovementASMC();
	Proc_Goto((ProcPtr*)proc, 0xF);
	return;	
}

/*
 * On Game Control 
 */
 
void ChangeControlledUnitASMC(struct FMUProc* proc){
	proc->FMUnit=GetUnitStructFromEventParameter(gEventSlots[1]);
	EnsureCameraOntoPosition(0,proc->FMUnit->xPos, proc->FMUnit->yPos);
	return;
}

void NewPlayerPhaseEvaluationFunc(ProcPtr* ParentProc){
	if( GetFreeMovementState() )
		Proc_StartBlocking(FreeMovementControlProc, ParentProc);
	else
		Proc_Goto(Proc_StartBlocking(gProcScr_PlayerPhase, ParentProc), 0x7);
	Proc_Break(ParentProc);
	return;
}
 
void NewMakePhaseControllerFunc(ProcPtr* ParentProc){
	const struct ProcCmd * pTmpProcCode = FreeMovementControlProc;
	if(0==GetFreeMovementState())
	{
		if( 0==gPlaySt.faction )
			pTmpProcCode=gProcScr_PlayerPhase;
		else
			pTmpProcCode=gProcScr_CpPhase;
	}
	Proc_StartBlocking(pTmpProcCode,ParentProc);
	Proc_Break(ParentProc);
	return;
}


/*
 * Inside Proc
 */
void pFMU_OnInit(struct FMUProc* proc){
	//vaild?
	if( 0 == proc->FMUnit )
		proc->FMUnit = gUnitArrayBlue;
	if( !( 1&(u32)(proc->FMUnit)>>0x11) )
		proc->FMUnit = gUnitArrayBlue;
	if( !( 1&(u32)(proc->FMUnit)>>0x19) )
		proc->FMUnit = gUnitArrayBlue;
	
	gActiveUnit = proc->FMUnit;
	return;
}


void pFMU_InitTimer(struct FMUProc* proc){
	proc->uTimer = 0;
	return;
}


void pFMU_CorrectCameraPosition(struct FMUProc* proc){
	EnsureCameraOntoPosition((ProcPtr*)proc, gActiveUnit->xPos, gActiveUnit->yPos);
}

extern struct KeyStatusBuffer sKeyStatusBuffer;

u8 FMU_ChkKeyForMUExtra(void){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if ( iKeyCur&0x10 )	//right
		return 1;
	if ( iKeyCur&0x20 )	//left
		return 0;
	if ( iKeyCur&0x40 )	//up
		return 3;
	if ( iKeyCur&0x80 )	//down
		return 2;
	return 0x10;	
}