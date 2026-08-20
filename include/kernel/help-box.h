#pragma once

#include "common-chax.h"

/* Hack */
enum NewHelpBoxType {
	NEW_HB_DEFAULT = 0,
	NEW_HB_COMBAT_ART_BKSEL,
	NEW_HB_WRANK_STATSCREEN,
};
extern int sHelpBoxType;

enum {
	HELP_BOX_MODE_VANILLA = 0,
	HELP_BOX_MODE_EXTENDED = 1,
	HELP_BOX_MODE_PAGED = 2,
};

#define HelpBoxModeExtended() \
	(gpKernelDesignerConfig->text_box_extension_layout == HELP_BOX_MODE_EXTENDED)
#define HelpBoxModePaged() \
	(gpKernelDesignerConfig->text_box_extension_layout == HELP_BOX_MODE_PAGED)

/* Stat screen / shop: avoid 0x6013000 — skill capacity circle overwrites +0x760 there. */
#define HelpBoxNeedsSafeVram() \
	(HelpBoxModeExtended() || HelpBoxModePaged())

struct HelpBoxPageState {
	u8 page;             /* 0-based */
	u8 page_count;
	u8 lines_per_page;   /* desc lines drawn on the current page */
	u8 pretext_lines;    /* draw pretext for the current page */
	u8 desc_lines;
	u8 page0_desc_lpp;   /* desc lines consumed on page 0 */
	u8 later_desc_lpp;   /* desc lines per page on page 1+ */
	u8 indicator_px;
};

extern struct HelpBoxPageState sHelpBoxPageState;

void HelpBoxResetPageState(void);
void HelpBoxStoreDescLineCount(int descHeightPx);
void HelpBoxFinalizePageState(int pretext_lines);
int HelpBoxCountDescLines(const char *str);
int HelpBoxDescLinesToSkip(void);
const char *HelpBoxSkipDescLines(const char *str, int linesToSkip);
void HelpBoxDrawPageIndicator(void);
void HelpBoxPutPageIndicatorSprites(int boxX, int boxY, int boxW);

/* From decomp */

extern struct ProcCmd ProcScr_Helpbox_bug_08A01678[];
int DrawHelpBoxWeaponLabels(int item);
void DrawHelpBoxWeaponStats(int item);
int DrawHelpBoxStaffLabels(int item);
void DrawHelpBoxSaveMenuLabels(void);
void DrawHelpBoxWeaponStats(int item);
void DrawHelpBoxSaveMenuStats(void);
