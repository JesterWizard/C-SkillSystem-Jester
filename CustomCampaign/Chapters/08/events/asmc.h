void RemoveInventories() {
    struct Unit * unit_ephraim = GetUnitFromCharId(CHARACTER_EPHRAIM);
    struct Unit * unit_forde = GetUnitFromCharId(CHARACTER_FORDE);
    struct Unit * unit_kyle = GetUnitFromCharId(CHARACTER_KYLE);
    
    for (int i = 0; i < 5; i++) {
        unit_ephraim->items[i] = 0;
        unit_forde->items[i] = 0;
        unit_kyle->items[i] = 0;
    }
};