#include "common-chax.h"
#include "kernel-lib.h"

void Banim_C03(struct Anim *anim)
{
	if (gpKernelDesignerConfig->c03_do_not_flush_efx_status != true)
	{
		if (GetUnitEfxDebuff(gAnims[0]) & 0xC)
			SetUnitEfxDebuff(gAnims[0], 0);

		if (GetUnitEfxDebuff(gAnims[1]) & 0xC)
			SetUnitEfxDebuff(gAnims[1], 0);

		if (GetUnitEfxDebuff(gAnims[2]) & 0xC)
			SetUnitEfxDebuff(gAnims[2], 0);

		if (GetUnitEfxDebuff(gAnims[3]) & 0xC)
			SetUnitEfxDebuff(gAnims[3], 0);
	}

	if (!(anim->state3 & ANIM_BIT3_BLOCKING)) {
		anim->state3 |= ANIM_BIT3_BLOCKING;
		if (GetAISLayerId(anim) == 0)
			NewEfxSpecalEffect(anim);
	}

	if (anim->state3 & ANIM_BIT3_BLOCKEND) {
		anim->state3 &= ~ANIM_BIT3_BLOCKING;
		anim->state3 &= ~ANIM_BIT3_BLOCKEND;
		anim->pScrCurrent = anim->pScrCurrent + 1;
	}
}
