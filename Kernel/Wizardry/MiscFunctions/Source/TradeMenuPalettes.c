#include "common-chax.h"
#include "bmtrade.h"

/*
 * The map trade screen's name banners (TradeMenu_InitUnitNameDisplay, MiscFunctions.c) use OBJ palette 15, the palette
 * of the map sprites of units that have acted (vanilla used 8). Nothing put it back when the trade closed: every
 * waited unit, the trader too once it chose Wait, was drawn with the banner's colours until the sprite palettes were
 * next reloaded. The trade proc's last step (ProcScr_TradeMenu, entry 19, MiscFunctions.event) now also reloads the
 * unit sprite palettes, in the same frame, after the faces and before the banners go with the proc.
 */
void TradeMenu_ClearDisplayAndRestorePalettes(ProcPtr proc)
{
	TradeMenu_ClearDisplay(proc);
	ApplyUnitSpritePalettes();
}
