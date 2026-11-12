#pragma once

#include "common-chax.h"

/* Map colours (Kernel/Wizardry/MapColors): the Options value gPlaySt.config.map_colors recolours the map tiles */
enum map_colors_mode {
	MAP_COLORS_VANILLA,
	MAP_COLORS_VIVID,
	MAP_COLORS_DEEP,

	MAP_COLORS_MODES
};

u16 MapColors_Transform(u16 color, int mode);
void MapColors_ApplyRange(u16 *pal, int count);
