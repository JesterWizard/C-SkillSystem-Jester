#include "FreeMU.h"

static inline bool IsPosInvaild(s8 x, s8 y){
	return( (x<0) & (x>gBmMapSize.x) & (y<0) & (y>gBmMapSize.y) );
}

static inline bool IsCharNotOnMap(struct Unit * unit){
	if(-1==unit->xPos)
		return 1;
	return 0;
}

static inline bool IsCharInvaild(struct Unit* unit){
	if(0==unit)
		return 1;
	if(0==unit->pCharacterData)
		return 1;
	return 0;
}





/*
 *
 *	In Core
 *
*/

void pFMU_MainLoop(struct FMUProc* proc){
	if(MuExists()){
		/* u8 iKeyDirec = FMU_CheckDirectionButtonPress();
		if(0!=iKeyDirec)
			*(gUnitMoveBuffer+0x4) = iKeyDirec; */
		return;
	}
	Proc_Goto((ProcPtr*)proc,0x1);
	return;
}


void pFMU_HanleContinueMove(struct FMUProc*){
	return;
}

extern struct KeyStatusBuffer sKeyStatusBuffer;

void pFMU_MoveUnit(struct FMUProc* proc){	//Label 1
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	s8 x = gActiveUnit->xPos;
	s8 y = gActiveUnit->yPos;
	
	//proc->xCur = x;
	//proc->xCur = y;
	//proc->xTo  = x;
	//proc->xTo  = y;
	
	if(0xF0&iKeyCur){
		if(iKeyCur&0x10) 		x++;
		else if(iKeyCur&0x20)	x--;
		else if(iKeyCur&0x40)	y--;
		else if(iKeyCur&0x80)	y++;
	}
	
		
	if( !IsPosInvaild(x,y) ){
		proc->xTo = x;
		proc->xTo = y;
	}
		
	if( FMU_CanUnitBeOnPos(gActiveUnit, x, y) ){
		if( !IsPosInvaild(x,y) )
			MuCtr_StartMoveTowards(gActiveUnit, x, y, 0x10, 0x10);
	}
	else
		Proc_Goto((ProcPtr*)proc,0x2);
	return;
}



void pFMU_HandleKeyMisc(struct FMUProc* proc){	//Label 2
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if(1&iKeyCur){ 			//Press A
		Proc_Goto((ProcPtr*)proc,0x4); 
		return;
		}			
	if(2&iKeyCur){ 			//Press B
		Proc_Goto((ProcPtr*)proc,0x5); 
		return;
		}	
	if(2&(iKeyCur>>0x8)){ 	//Press L
		Proc_Goto((ProcPtr*)proc,0x6); 
		return;
		}
	if(1&(iKeyCur>>0x8)){ 	//Press R
		Proc_Goto((ProcPtr*)proc,0x7); 
		return;
		}
	if(4&iKeyCur){ 			//Press Select
		Proc_Goto((ProcPtr*)proc,0x8); 
		return;
		}
	if(8&iKeyCur){ 			//Press Start
		Proc_Goto((ProcPtr*)proc,0x9); 
		return;
		}
	return;
}

void pFMU_HandleSave(struct FMUProc* proc){	//KeyPress Default
	if( TimerDelay < ++proc->uTimer ){
		Proc_Goto((ProcPtr*)proc,0xE);
		proc->uTimer=0;
	}
	return;
}


void pFMU_PressA(struct FMUProc* proc){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if( 0 == (1&iKeyCur) )
		return;
	
	ButtonFunc* it = &FMU_FunctionList_OnPressA[0];
	while(*it)
		if( (*it++)(proc) )
			return;
	return;
}


void pFMU_PressB(struct FMUProc* proc){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if( 0 == (2&iKeyCur) )
		return;
	
	ButtonFunc* it = &FMU_FunctionList_OnPressB[0];
	while(*it)
		if( (*it++)(proc) )
			return;
	return;
}

void pFMU_PressL(struct FMUProc* proc){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if( 0 == (2&iKeyCur>>0x8) )
		return;
	
	ButtonFunc* it = &FMU_FunctionList_OnPressL[0];
	while(*it)
		if( (*it++)(proc) )
			return;
	return;
}

void pFMU_PressR(struct FMUProc* proc){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if( 0 == (1&iKeyCur>>0x8) )
		return;
	
	ButtonFunc* it = &FMU_FunctionList_OnPressR[0];
	while(*it)
		if( (*it++)(proc) )
			return;
	return;
}

void pFMU_PressStart(struct FMUProc* proc){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if( 0 == (8&iKeyCur) )
		return;
	
	ButtonFunc* it = &FMU_FunctionList_OnPressStart[0];
	while(*it)
		if( (*it++)(proc) )
			return;
	return;
}

void pFMU_PressSelect(struct FMUProc* proc){
	u16 iKeyCur = sKeyStatusBuffer.heldKeys;
	if( 0 == (4&iKeyCur) ) 			//Press Select
		return;

	ButtonFunc* it = &FMU_FunctionList_OnPressSelect[0];
	while(*it)
		if( (*it++)(proc) )
			return;
	return;
}

u16 NewGetCameraCenteredX(int x)
{
    int result = gBmSt.camera.x;

    if (gBmSt.camera.x + CAMERA_MARGIN_LEFT > x)
    {
        result = x - CAMERA_MARGIN_LEFT < 0 ? 0 : x - CAMERA_MARGIN_LEFT;
    }

    if (gBmSt.camera.x + CAMERA_MARGIN_RIGHT < x)
    {
        result = x - CAMERA_MARGIN_RIGHT > gBmSt.cameraMax.x ? gBmSt.cameraMax.x : x - CAMERA_MARGIN_RIGHT;
    }

    return result;
}

u16 NewGetCameraCenteredY(int y)
{
    int result = gBmSt.camera.y;

    if (gBmSt.camera.y + CAMERA_MARGIN_TOP > y)
    {
        result = y - CAMERA_MARGIN_TOP < 0 ? 0 : y - CAMERA_MARGIN_TOP;
    }

    if (gBmSt.camera.y + CAMERA_MARGIN_BOTTOM < y)
    {
        result = y - CAMERA_MARGIN_BOTTOM > gBmSt.cameraMax.y ? gBmSt.cameraMax.y : y - CAMERA_MARGIN_BOTTOM;
    }

    return result;
}