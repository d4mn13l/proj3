#include <3ds.h>

#include <stdio.h>

#include "game.h"
#include "map.h"
#include "player.h"
#include "renderer.h"
#include "shared.h"
#include "util.h"


#define MAP_FILE_PATH "romfs:/map.txt"


int main() {
	romfsInit();
	g_log_file = fopen("proj3.log", "w");

	gfxInitDefault();
	gfxSetDoubleBuffering(GFX_TOP, true);
	consoleInit(GFX_BOTTOM, NULL);


	FILE *tex_f = fopen("romfs:/wall_textures.ppm", "r");
	ASSERT_ALWAYS(tex_f != NULL);
	tex_atlas_load(&g_game.ta, tex_f);
	fclose(tex_f);

	// load map
	FILE *map_file = fopen(MAP_FILE_PATH, "r");
	ASSERT_ALWAYS(map_file != NULL);
	map_load(&g_game.map, map_file);
	fclose(map_file);
	map_file = NULL;
	map_print_debug_info(&g_game.map);

	player_init(&g_game.player, &g_game.map);

	osTickCounterStart(&g_timer);
	// float delta;
	// time that the last tick took in s
	
	g_game.state = GAME_STATE_PLAYING;
	g_game.state_args = NULL;
	// game.state = GAME_STATE_TEXT;
	// game.state_args = (void *) lore[0];

	while (aptMainLoop()) {
		if (game_tick()) break;
	}
	
	
	map_free(&g_game.map);

	if (g_log_file) fclose(g_log_file);

	romfsExit();
	gfxExit();

	return 0;
}
