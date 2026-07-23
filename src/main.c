#include <3ds.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "map.h"
#include "maths.h"
#include "shared.h"
#include "util.h"


int main() {
	romfsInit();
	g_log_file = fopen("proj3.log", "w");
	ASSERT_ALWAYS(g_log_file != NULL);
	fprintf(g_log_file, "%f\n\n", vec3_angle((vec3_t) {1, 0, 0}, (vec3_t) {1, 1, 0}));

	srand(time(NULL));

	gfxInitDefault();
	gfxSetDoubleBuffering(GFX_TOP, true);
	consoleInit(GFX_BOTTOM, NULL);

	game_init("romfs:/map_upstairs.txt");
	// game_init("romfs:/map_mine.txt");

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
