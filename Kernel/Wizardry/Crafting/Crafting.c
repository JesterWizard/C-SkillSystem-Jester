#include "gbafe.h"

extern u8 RecipeTable[255][6];
extern u16 InsufficientIngredientsRText_Link;
extern u16 AlchemyLevelTextID_Link;
extern u16 ExperienceTextID_Link;
extern u16 AlchemyLevelTooLowRText_Link;
extern u16 ResultQualityName_Link;

void DrawExternalSynthesisUI(struct Proc* proc) {
	ResetText();

	DrawUiFrame(gBG1TilemapBuffer, 1, 5, 13, 14, 0, 0);
	for(int i = 0; i < 5; i++) {
		InitText(&gPrepItemTexts[i], 9);
		PutDrawText(&gPrepItemTexts[i], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 6 + (2 * (i))), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(0x0001));
	}
	InitText(&gPrepItemTexts[5], 13);
	PutDrawText(&gPrepItemTexts[5], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 16), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(0x0001));
	
	DrawUiFrame(gBG1TilemapBuffer, 1, 1, 13, 4, 0, 0);
	
	InitText(&gPrepItemTexts[6], 4);
	PutDrawText(&gPrepItemTexts[6], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 2), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(AlchemyLevelTextID_Link));
	PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 2), TEXT_COLOR_SYSTEM_GOLD, gActiveUnit->level);
	
	InitText(&gPrepItemTexts[7], 3);
	PutDrawText(&gPrepItemTexts[7], TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 2), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(ExperienceTextID_Link));
	PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 2), TEXT_COLOR_SYSTEM_GOLD, gActiveUnit->exp);
}

void CallSynthesisMenuASMC(struct Proc* proc)
{
    ClearBg0Bg1();
    SetDispEnable(1, 1, 1, 1, 1);
    SetTextFont(0);
    InitSystemTextFont();
    LoadUiFrameGraphics();
	ResetIconGraphics_();
	
	struct MenuDef* menu = (struct MenuDef*) gEventSlots[0x1];
	
    StartMenu(menu, proc);
}

u8 SynthesisMenuBPress(struct MenuProc* menu, struct MenuItemProc* menuItem)
{
	u8 recipe = menuItem->def->overrideId;
	for(int i = 0; (i < 5 && RecipeTable[recipe][i+1]); i++) {
		ClearIconGfx(GetItemIconId(RecipeTable[recipe][i+1]));
		ClearText(&gPrepItemTexts[i]);
	}
	
	ClearText(&gPrepItemTexts[5]);
	gEventSlots[0xC] = 0;
    return MENU_ACT_SKIPCURSOR | MENU_ACT_CLEAR | MENU_ACT_END | MENU_ACT_SND6B;
}

bool IsRecipeFulfilled(u8 recipe) {
	if(gActiveUnit->level < RecipeTable[recipe][0]) {
		return false;
	}
	
	for(int i = 1; (i < 6 && RecipeTable[recipe][i]); i++) {
		bool matchFound = false;
		
		for(int j = 0; (j < 5 && GetItemIndex(gActiveUnit->items[j])); j++) {
			if(RecipeTable[recipe][i] == GetItemIndex(gActiveUnit->items[j])) {
				matchFound = true;
				break;
			}
		}
		
		if(matchFound == false) {
			return false;
		}
	}
	
	return true;
}

u8 RecipeOptionUsability(struct MenuItemDef* def, int number) {
	u8 recipe = def->overrideId;
	if(IsRecipeFulfilled(recipe)) {
		return MENU_ENABLED;
	}
	else {
		return MENU_DISABLED;
	}
}


int DrawRecipeOption(struct MenuProc* menu, struct MenuItemProc* menuItem) {
	int color;
	u8 recipe = menuItem->def->overrideId;
	
	if(recipe == 0 || recipe == 0xFF) {
		color = TEXT_COLOR_SYSTEM_WHITE;
		PutDrawText(&menuItem->text, TILEMAP_LOCATED(gBG0TilemapBuffer, menuItem->xTile + 1, menuItem->yTile), color, 0, 0, GetStringFromIndex(menuItem->def->nameMsgId));
	}
	else {
		color = (gActiveUnit->level >= RecipeTable[recipe][0]) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY;
		PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, menuItem->xTile + 1, menuItem->yTile), color, RecipeTable[recipe][0]);
		DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, menuItem->xTile + 2, menuItem->yTile), GetItemIconId(recipe), 0x4000);
		
		color = (IsRecipeFulfilled(recipe)) ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY;
		PutDrawText(&menuItem->text, TILEMAP_LOCATED(gBG0TilemapBuffer, menuItem->xTile + 4, menuItem->yTile), color, 0, 0, GetStringFromIndex(menuItem->def->nameMsgId));
	}
    return 0;
}

u8 GetProductQuality(u8 recipe) {
	u8 numIngredients = 0;
	u8 rawQuality = 0;
	
	for(int i = 1; (i < 6 && RecipeTable[recipe][i]); i++) {
		for(int j = 0; (j < 5 && GetItemIndex(gActiveUnit->items[j])); j++) {
			if(RecipeTable[recipe][i] == GetItemIndex(gActiveUnit->items[j])) {
				numIngredients++;
				rawQuality += GetItemUses(gActiveUnit->items[j]);
				break;
			}
		}
	}
	
	return rawQuality / numIngredients;
}

int RecipeSwitchIn(struct MenuProc* menu, struct MenuItemProc* menuItem) {
	
	u8 recipe = menuItem->def->overrideId;
	u8 quality = 0;
	u8 color = TEXT_COLOR_SYSTEM_GRAY;
	
	if(recipe != 0 && recipe != 0xFF) {
		for(int i = 1; (i < 6 && RecipeTable[recipe][i]); i++) {
			
			DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 6 + (2 * (i-1))), GetItemIconId(RecipeTable[recipe][i]), 0x4000);
			for(int j = 0; j < 5 && gActiveUnit->items[j]; j++) {
				if(RecipeTable[recipe][i] == GetItemIndex(gActiveUnit->items[j])) {
					color = TEXT_COLOR_SYSTEM_WHITE;
					quality = GetItemUses(gActiveUnit->items[j]);
					break;
				}
			}
			PutDrawText(&gPrepItemTexts[i-1], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 6 + (2 * (i-1))), color, 0, 0, GetItemName(RecipeTable[recipe][i]));
			if(color == TEXT_COLOR_SYSTEM_WHITE) {
				Text_InsertDrawNumberOrBlank(&gPrepItemTexts[i-1], 64, TEXT_COLOR_SYSTEM_BLUE, quality);
				//Text_InsertDrawNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 6 + (2 * (i-1))), TEXT_COLOR_SYSTEM_BLUE, quality);
			}
			
			color = TEXT_COLOR_SYSTEM_GRAY;
		}
		
		color = (IsRecipeFulfilled(recipe)) ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY;
		PutDrawText(&gPrepItemTexts[5], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 16), color, 0, 0, GetStringFromIndex(ResultQualityName_Link));
		if(IsRecipeFulfilled(recipe)) {
			color = TEXT_COLOR_SYSTEM_BLUE;
			Text_InsertDrawNumberOrBlank(&gPrepItemTexts[5], 80, color, GetProductQuality(recipe));
		}
	}
    return 0;
}

int RecipeSwitchOut(struct MenuProc* menu, struct MenuItemProc* menuItem) {
	u8 recipe = menuItem->def->overrideId;
	for(int i = 0; (i < 5 && RecipeTable[recipe][i+1]); i++) {
		ClearIconGfx(GetItemIconId(RecipeTable[recipe][i+1]));
		ClearText(&gPrepItemTexts[i]);
	}
	
	ClearText(&gPrepItemTexts[5]);
	//BG_Fill(gBG0TilemapBuffer, 0);
	//BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
    return 0;
}

void RemoveUsedMaterials(u8 recipe) {
	for(int i = 1; (i < 6 && RecipeTable[recipe][i]); i++) {
		for(int j = 0; (j < 5 && GetItemIndex(gActiveUnit->items[j])); j++) {
			if(RecipeTable[recipe][i] == GetItemIndex(gActiveUnit->items[j])) {
				UnitRemoveItem(gActiveUnit, j);
				break;
			}
		}
	}	
}

u8 OnRecipeSelect(struct MenuProc* menu, struct MenuItemProc* menu_item) {
	u8 recipe = menu_item->def->overrideId;
	if(menu_item->availability == MENU_ENABLED) {
		if(recipe == 0) {
			gEventSlots[0xC] = 1;
			return MENU_ACT_CLEAR | MENU_ACT_SND6A | MENU_ACT_END | MENU_ACT_SKIPCURSOR;
		}
		else if(recipe == 0xFF) {
			gEventSlots[0xC] = 2;
			return MENU_ACT_CLEAR | MENU_ACT_SND6A | MENU_ACT_END | MENU_ACT_SKIPCURSOR;
		}
		
		gEventSlots[0xC] = 0;
		
		u8 quality = GetProductQuality(recipe);
		RemoveUsedMaterials(recipe);
		u16 item = recipe + (quality << 8);
		NewPopup_ItemGot(menu->proc_parent, gActiveUnit, item);
		
		return MENU_ACT_CLEAR | MENU_ACT_SND6A | MENU_ACT_END | MENU_ACT_SKIPCURSOR;
		//return MENU_ACT_CLEAR | MENU_ACT_SND6A | MENU_ACT_END | MENU_ACT_SKIPCURSOR;
	}
	
	if(gActiveUnit->level < RecipeTable[recipe][0]) {
		MenuFrozenHelpBox(menu, AlchemyLevelTooLowRText_Link);
	}
	else {
		MenuFrozenHelpBox(menu, InsufficientIngredientsRText_Link);
	}
	
	return MENU_ACT_SND6B;
}