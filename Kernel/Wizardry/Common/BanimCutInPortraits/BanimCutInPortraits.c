#include "common-chax.h"

#define PORTRAIT_OBJ_VRAM (OBJ_VRAM0 + 0x3000)
#define PORTRAIT_PAL_SLOT 0x10  // OBJ palette slot

struct ProcEfxCriticalPortrait {
    PROC_HEADER;

    struct Anim * anim;
    int timer;

    int x;
    int target_x;

    int slide_in_frames;
    int hold_frames;
    int slide_out_frames;
};


LYN_REPLACE_CHECK(efxCriricalEffectMain);
void efxCriricalEffectMain(struct ProcEfx * proc)
{
    int time = ++proc->timer;

    if (time == 1) {
        NewEfxCriricalEffectBG(proc->anim);
        NewEfxCriricalEffectBGCOL(proc->anim);
        return;
    }

    if (time == 0x11)
        Proc_Break(proc);
}

void DrawCriticalPortrait(struct ProcEfxCriticalPortrait * proc)
{
    StartFace(1, CHARACTER_EIRIKA, 208, 80, FACE_DISP_KIND(FACE_96x80));
}

void EfxCriticalPortraitMain(struct ProcEfxCriticalPortrait * proc)
{
    int total_in  = proc->slide_in_frames;
    int total_hold = proc->hold_frames;
    int total_out = proc->slide_out_frames;

    proc->timer++;

    if (proc->timer <= total_in) {
        // Slide in
        int dx = (proc->target_x - proc->x) / (total_in - proc->timer + 1);
        proc->x += dx;
    }
    else if (proc->timer <= total_in + total_hold) {
        // Hold (do nothing)
    }
    else if (proc->timer <= total_in + total_hold + total_out) {
        // Slide out
        int dx = (GetAnimPosition(proc->anim) == EKR_POS_L) ? -6 : 6;
        proc->x += dx;
    }
    else {
        Proc_Break(proc);
        return;
    }

    DrawCriticalPortrait(proc);
}

const struct ProcCmd ProcScr_EfxCriticalPortrait[] = {
    PROC_NAME("EfxCriticalPortrait"),
    PROC_REPEAT(EfxCriticalPortraitMain),
    PROC_END
};

void NewEfxCriticalPortraitSlide(struct Anim * anim, struct Proc * parent)
{
    struct ProcEfxCriticalPortrait * proc;

    proc = Proc_StartBlocking(ProcScr_EfxCriticalPortrait, parent);
    proc->anim = anim;
    proc->timer = 0;

    // Speed control (frames)
    proc->slide_in_frames  = 8;
    proc->hold_frames      = 12;
    proc->slide_out_frames = 8;

    if (GetAnimPosition(anim) == EKR_POS_L) {
        proc->x = -80;
        proc->target_x = 8;
    } else {
        proc->x = 240;
        proc->target_x = 240 - 88;
    }
}

LYN_REPLACE_CHECK(NewEfxPierceCritical);
void NewEfxPierceCritical(struct Anim * anim)
{
    struct ProcEfx * proc;
    SpellFx_ClearBG1Position();

    proc = Proc_Start(ProcScr_efxCriricalEffect, PROC_TREE_3);
    proc->anim = anim;
    proc->timer = 0;

#ifdef CONFIG_PORTRAIT_BANIM_CUT_IN_CRITICAL
    // NEW: spawn portrait as CHILD
    NewEfxCriticalPortraitSlide(anim, (struct Proc *)proc);
#endif
}