#include "game.h"

#include <3ds.h>
#include <stdlib.h>

#include "3ds/console.h"
#include "3ds/gfx.h"
#include "3ds/services/gspgpu.h"
#include "3ds/svc.h"
#include "map.h"
#include "text.h"
#include "maths.h"
#include "ppm.h"
#include "renderer.h"
#include "keymap.h"
#include "shared.h"
#include "util.h"


int game_tick_play() {
	float fov = deg_to_rad(60);
	u64 tick_start = svcGetSystemTick();
	printf("\x1b[38;1H%f", g_delta);

	hidScanInput();
	u32 keys_down = hidKeysDown();
	if (keys_down & KEY_START) return EXIT_FAILURE;

	u32 keys_held = hidKeysHeld();
	player_handle_input(g_game.player, g_game.map, keys_held, g_delta);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
	// rendering
	image_t fb;
	fb.w = 400;
	fb.h = 240;
	fb.pixels = (colour_t*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	float cur_time = (float) (tick_start / CPU_TICKS_PER_MSEC) / 1000;
	u8 dim_factor = 4;
	render(&fb, g_game.map, g_game.player->pos.x, g_game.player->pos.y, fov, 
		g_game.player->rotation, g_game.ta, shade_dark, (void *) &dim_factor,
		2);
	

		g_delta = (float) (svcGetSystemTick() - tick_start) / (CPU_TICKS_PER_MSEC * 1000);

	return EXIT_SUCCESS;
}


int game_tick_text() {
	char c = *(char *) g_game.state_args;
	printf("%c", c);
	if (c == '\n') svcSleepThread(LORE_NEWLINE_DELAY_MS * 1000000);
	else svcSleepThread(LORE_CHAR_DELAY_MS * 1000000);
	gfxFlushBuffers();
	gfxScreenSwapBuffers(GFX_BOTTOM, false);
	// gspWaitForVBlank();
	

	if (c == '\0') {
		puts("\n");
		game_set_state(GAME_STATE_PLAYING, NULL);
	} else {
		g_game.state_args = (char *)g_game.state_args + 1;
	}

	hidScanInput();
	u32 keys_held = hidKeysHeld();
	if (keys_held & KEY_SKIP_TEXT) {
		puts((char *) g_game.state_args);
		game_set_state(GAME_STATE_PLAYING, NULL);
	}

	return EXIT_SUCCESS;
}




void game_interact(prop_t *prop) {
	switch (prop->interactable_type){
	case INTERACTABLE_TYPE_NONE:
		// ???
		return;
	case INTERACTABLE_TYPE_LORE:
		game_set_state(GAME_STATE_TEXT, (void *) text[prop->data[0]]);
	}
}

int game_tick() {
	switch (g_game.state) {
	case GAME_STATE_PLAYING:
		return game_tick_play();
	case GAME_STATE_TEXT:
		return game_tick_text();
	default:
		UNREACHABLE("illegal game state value");
	}
	
}


void game_set_state(int state, void *state_args) {
	// state exit stuff
	switch (state) {
		
	}
	g_game.state = state;
	g_game.state_args = state_args;

	switch (state) {
	case GAME_STATE_TEXT:
		consoleClear();
	}
}
