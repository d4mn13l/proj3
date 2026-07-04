#include <3ds.h>

#include <stdio.h>

#include "game.h"
#include "map.h"
#include "shared.h"


#define MAP_FILE_PATH "romfs:/map.txt"


int main() {
	romfsInit();
	g_log_file = fopen("proj3.log", "w");

	gfxInitDefault();
	gfxSetDoubleBuffering(GFX_TOP, true);
	consoleInit(GFX_BOTTOM, NULL);

	game_init();

	osTickCounterStart(&g_timer);

	while (aptMainLoop()) {
		if (game_tick()) break;
	}

	game_deinit();
	
	
	map_free(&g_game.map);

	if (g_log_file) fclose(g_log_file);

	romfsExit();
	gfxExit();

	return 0;
}
