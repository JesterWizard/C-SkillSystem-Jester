#ifndef GUARD_UNITINFOWINDOW_H
#define GUARD_UNITINFOWINDOW_H

enum { LINES_MAX = 5 };
struct UnitInfoWindowProc {
    /* 00 */ PROC_HEADER;

    /* 2C */ struct Unit* unit;

    /* 30 */ struct Text name;
    /* 38 */ struct Text lines[LINES_MAX];

    /* 60 */ u8 x;
    /* 61 */ u8 y;
    /* 62 */ u8 xUnitSprite;
    /* 63 */ u8 xNameText;
};

void UnitInfoWindow_OnLoop(struct UnitInfoWindowProc* proc);
struct UnitInfoWindowProc* NewUnitInfoWindow(ProcPtr parent);
void UnitInfoWindow_PositionUnitName(struct UnitInfoWindowProc* proc);
struct UnitInfoWindowProc *UnitInfoWindow_DrawBase(struct UnitInfoWindowProc *proc, struct Unit *unit, int x, int y, int width, int lines);
int GetUnitInfoWindowX(struct Unit *unit, int width);
void DrawUnitHpText(struct Text* text, struct Unit* unit);
void DrawUnitConText(struct Text* text, struct Unit* unit);
void DrawUnitAidText(struct Text* text, struct Unit* unit);
void PutUnitAidIconForTextAt(struct Unit* unit, int x, int y);
// ??? DrawUnitStatusText(???);
void DrawUnitResChangeText(struct Text* text, struct Unit* unit, int bonus);
// ??? DrawUnitResUnkText(???);
// ??? DrawAccuracyText(???);
void StartUnitInventoryInfoWindow(ProcPtr);
void RefreshUnitInventoryInfoWindow(struct Unit*);
void RefreshUnitStealInventoryInfoWindow(struct Unit*);
void RefreshHammerneUnitInfoWindow(struct Unit*);
void StartUnitHpInfoWindow(ProcPtr);
void RefreshUnitHpInfoWindow(struct Unit*);
void StartUnitHpStatusInfoWindow(ProcPtr proc);
void RefreshUnitHpStatusInfoWindow(struct Unit* unit);
void StartUnitResChangeInfoWindow(ProcPtr proc);
void RefreshUnitResChangeInfoWindow(struct Unit* unit);
void StartUnitStaffOffenseInfoWindow(ProcPtr proc);
void RefreshUnitStaffOffenseInfoWindow(struct Unit* unit, int number);
// ??? StartUnitRescueInfoWindowsCore(???);
void RefreshUnitTakeRescueInfoWindows(ProcPtr);
void RefreshUnitRescueInfoWindows(struct Unit*);
void RefreshUnitTakeInfoWindows(struct Unit*);
void StartUnitGiveInfoWindows(ProcPtr);
void RefreshUnitGiveInfoWindows(struct Unit*);


#endif // GUARD_UNITINFOWINDOW_H
