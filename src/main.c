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

	// load map
	FILE *map_file = fopen(MAP_FILE_PATH, "r");
	ASSERT_ALWAYS(map_file != NULL);
	map_t map;
	map_load(&map, map_file);
	fclose(map_file);
	map_file = NULL;
	map_print_debug_info(&map);

	tex_atlas_t ta;
	FILE *tex_f = fopen("romfs:/wall_textures.ppm", "r");
	ASSERT_ALWAYS(tex_f != NULL);
	tex_atlas_load(&ta, tex_f, 8, 2);
	fclose(tex_f);

	player_t player;
	player_init(&player, &map);

	TickCounter timer;
	osTickCounterStart(&timer);
	// float delta;
	// time that the last tick took in s

	g_game.map = &map;
	g_game.player = &player;
	g_game.ta = &ta;
	
	g_game.state = GAME_STATE_PLAYING;
	g_game.state_args = NULL;
	// game.state = GAME_STATE_TEXT;
	// game.state_args = (void *) lore[0];

	while (aptMainLoop()) {
		if (game_tick()) break;
	}
	
	
	map_free(&map);

	if (g_log_file) fclose(g_log_file);

	romfsExit();
	gfxExit();

	return 0;
}
