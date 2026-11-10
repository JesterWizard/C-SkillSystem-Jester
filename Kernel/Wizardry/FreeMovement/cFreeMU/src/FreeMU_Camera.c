#include "FreeMU.h"
#include "common-chax.h"

/*
 * Free movement camera: the camera glides to keep the controlled unit near the middle of the screen, following its
 * map sprite pixel by pixel while it walks, instead of standing still until the unit nears an edge and then scrolling
 * a block (vanilla EnsureCameraOntoPosition). It is a child of the free movement proc, started from its idle label
 * (FreeMovementInternal.event), and stands aside while that proc is blocked (events, menus, minigames) or a vanilla
 * camera move runs.
 */

/* Each frame the camera covers 1/FMU_CAMERA_EASE of the way, at most FMU_CAMERA_MAX_STEP pixels */
#define FMU_CAMERA_EASE 4
#define FMU_CAMERA_MAX_STEP 6

/* Where the unit's 16x16 tile sits on screen: the middle */
#define FMU_CAMERA_X_OFFSET (DISPLAY_WIDTH / 2 - 8)
#define FMU_CAMERA_Y_OFFSET (DISPLAY_HEIGHT / 2 - 8)

extern struct ProcCmd CONST_DATA ProcScr_Mu[];
extern struct ProcCmd CONST_DATA ProcScr_CamMove[];

struct FmuCameraProc {
	PROC_HEADER;
};

/* The unit's map position in pixels: its walking sprite's while it moves, else its tile's */
static void FmuCamera_UnitPixel(struct Unit *unit, int *x, int *y)
{
	struct MuProc *mu = Proc_Find(ProcScr_Mu);

	while (mu) {
		if (mu->unit == unit) {
			*x = (mu->x_q4 + mu->x_offset_q4) >> MU_SUBPIXEL_PRECISION;
			*y = (mu->y_q4 + mu->y_offset_q4) >> MU_SUBPIXEL_PRECISION;
			return;
		}
		mu = Proc_FindAfter(ProcScr_Mu, (struct Proc *)mu);
	}

	*x = unit->xPos * 16;
	*y = unit->yPos * 16;
}

static int FmuCamera_Step(int from, int to)
{
	int step = (to - from) / FMU_CAMERA_EASE;

	if (step == 0 && to != from)
		step = to > from ? 1 : -1;
	if (step > FMU_CAMERA_MAX_STEP)
		step = FMU_CAMERA_MAX_STEP;
	if (step < -FMU_CAMERA_MAX_STEP)
		step = -FMU_CAMERA_MAX_STEP;
	return from + step;
}

static int FmuCamera_Clamp(int value, int max)
{
	if (value > max)
		value = max;
	return value < 0 ? 0 : value;
}

static void FmuCamera_Loop(struct FmuCameraProc *proc)
{
	struct Proc *fmu = proc->proc_parent;
	struct Unit *unit = gActiveUnit;
	int x, y;

	if (fmu->proc_lockCnt != 0 || Proc_Find(ProcScr_CamMove))
		return;
	if (!unit || !unit->pCharacterData)
		return;

	FmuCamera_UnitPixel(unit, &x, &y);
	x = FmuCamera_Clamp(x - FMU_CAMERA_X_OFFSET, gBmSt.cameraMax.x);
	y = FmuCamera_Clamp(y - FMU_CAMERA_Y_OFFSET, gBmSt.cameraMax.y);

	gBmSt.camera.x = FmuCamera_Step(gBmSt.camera.x, x);
	gBmSt.camera.y = FmuCamera_Step(gBmSt.camera.y, y);
}

static const struct ProcCmd ProcScr_FmuCamera[] = {
	PROC_NAME("FmuCamera"),
	PROC_REPEAT(FmuCamera_Loop),
	PROC_END,
};

/* Free movement proc, idle label: start the camera once (it ends with the free movement proc) */
void pFMU_StartCamera(struct FMUProc *proc)
{
	if (!Proc_Find(ProcScr_FmuCamera))
		Proc_Start(ProcScr_FmuCamera, proc);
}
