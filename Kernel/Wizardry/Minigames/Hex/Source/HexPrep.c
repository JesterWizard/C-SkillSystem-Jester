#include "common-chax.h"
#include "kernel-lib.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

/* Called from the prep menu (AtMenu.c, AtMenu_StartSubmenu). A long call into the WIZARDRY_CUSTOM region. */
void StartHexScreen_FromPrep(struct ProcAtMenu *pproc)
{
	HexScreen_SetDistrictDeck(HEX_NO_DECK);
	Proc_StartBlocking(ProcScr_HexScreen, pproc);
}

#endif /* CONFIG_HEX_CARDS */
