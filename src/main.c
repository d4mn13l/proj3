#include <3ds.h>

#include <stdio.h>
#include <string.h>

#include "3ds/os.h"
#include "3ds/svc.h"
#include "map.h"
#include "maths.h"
#include "player.h"
#include "ppm.h"
#include "renderer.h"
#include "shared.h"
#include "util.h"

#define MAP_FILE_PATH "romfs:/map.txt"


int main() {
	float fov = deg_to_rad(60);
	
	romfsInit();
	gfxInitDefault();
	gfxSetDoubleBuffering(GFX_TOP, true);
	consoleInit(GFX_BOTTOM, NULL);

	// load map
	FILE *map_file = fopen(MAP_FILE_PATH, "r");
	ASSERT_ALWAYS(map_file != NULL, __LINE__, __FILE__);
	map_t map;
	map_load(&map, map_file);
	fclose(map_file);
	map_file = NULL;
	map_print_debug_info(&map);

	tex_atlas_t ta;
	FILE *tex_f = fopen("romfs:/wall_textures.ppm", "r");
	ASSERT_ALWAYS(tex_f != NULL, __LINE__, __FILE__);
	tex_atlas_load(&ta, tex_f, 8, 1);
	fclose(tex_f);

	player_t player;
	player_init(&player, &map);

	TickCounter timer;
	osTickCounterStart(&timer);
	// float delta;
	// time that the last tick took in s

	while (aptMainLoop()) {
		u64 tick_start = svcGetSystemTick();

		hidScanInput();
		u32 keys_down = hidKeysDown();
		if (keys_down & KEY_START) break;

		u32 keys_held = hidKeysHeld();
		player_handle_input(&player, &map, keys_held, delta);

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		// rendering
		image_t fb;
		fb.w = 400;
		fb.h = 240;
		fb.pixels = (colour_t*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		float cur_time = (float) (tick_start / CPU_TICKS_PER_MSEC) / 1000;
		render(&fb, &map, player.pos.x, player.pos.y, fov,
			player.rotation, &ta, shade_blink, (void *) &cur_time);
		

		delta = (float) (svcGetSystemTick() - tick_start) / (CPU_TICKS_PER_MSEC * 1000);
	}
	
	
	map_free(&map);

	romfsExit();
	gfxExit();

	return 0;
}
