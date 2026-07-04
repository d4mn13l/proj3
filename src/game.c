#include "game.h"

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3ds/os.h"
#include "map.h"
#include "text.h"
#include "maths.h"
#include "ppm.h"
#include "renderer.h"
#include "keymap.h"
#include "shared.h"
#include "util.h"



void game_init() {
	FILE *tex_f = fopen("romfs:/wall_textures.ppm", "r");
	ASSERT_ALWAYS(tex_f != NULL);
	tex_atlas_load(&g_game.ta, tex_f);
	fclose(tex_f);

	// load map
	FILE *map_file = fopen("romfs:/map.txt", "r");
	ASSERT_ALWAYS(map_file != NULL);
	map_load(&g_game.map, map_file);
	fclose(map_file);
	map_file = NULL;
	map_print_debug_info(&g_game.map);

	player_init(&g_game.player, &g_game.map);

	creature_init(&g_game.creature, 11.5, 16.5);

	g_game.state = GAME_STATE_PLAYING;
	g_game.state_args = NULL;
}


void game_deinit() {
	map_free(&g_game.map);
}



void game_draw_bottom_screen() {
	consoleClear();
	printf("\x1b[27;1H frame time: %f", g_delta);
	printf("\x1b[28;1H creature pos: %f, %f", g_game.creature.pos.x, g_game.creature.pos.y);
	printf("\x1b[24;1H creature chase target: %f, %f", g_game.creature.chase_target.x, g_game.creature.chase_target.y);
	printf("\x1b[26;1H player pos: %f, %f", g_game.player.pos.x, g_game.player.pos.y);

	sprite_t *interactable = map_get_interactable(&g_game.map,
		g_game.player.pos.x, g_game.player.pos.y);
	if (interactable != NULL) {
		char *action;
		switch (interactable->interactable_type) {
		case INTERACTABLE_TYPE_NONE:
			action = "do nothing????";
			break;
		case INTERACTABLE_TYPE_LORE:
			action = "read";
			break;
		case INTERACTABLE_TYPE_PICKUP:
			action = "pick up";
			break;
		case INTERACTABLE_TYPE_DOOR_SWITCH:
			if (map_get_cell(&g_game.map, interactable->data[0],
				interactable->data[1])->tex == CELL_TEX_EMPTY) {			
				action = "close door";
			} else {
				action = "open door";
			}
			break;
		default:
			action = "do something";
		}
		printf("\x1b[29;5H(press Y to %s)", action);
	}
}


int game_tick_play() {
	float fov = deg_to_rad(60);

	hidScanInput();
	u32 keys_down = hidKeysDown();
	if (keys_down & KEY_START) return EXIT_FAILURE;

	game_draw_bottom_screen();

	player_tick(&g_game.player);
	creature_tick(&g_game.creature);
	
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
	// rendering
	image_t fb;
	fb.w = 400;
	fb.h = 240;
	fb.pixels = (colour_t*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);


	// calculate blink frequency

	float distance = vec3_length(
		vec3_sub(g_game.player.pos, g_game.creature.pos));

	float ratio = fminf(1, distance / BLINK_MAX_DISTANCE);
	ratio *= ratio;
	ratio = 1;
	
	render(&fb, &g_game.map, g_game.player.pos.x, g_game.player.pos.y, fov, 
		// g_game.player.rotation, &g_game.ta, shade_dark, (void *) &dim_factor,
		g_game.player.rotation, &g_game.ta, shade_blink, (void *) &ratio,
		2);
	


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
	
	hidScanInput();
	u32 keys_held = hidKeysHeld();
	
	if (c == '\0') {
		if (keys_held & KEY_INTERACT)
			game_set_state(GAME_STATE_PLAYING, NULL, NULL);
	} else {
		g_game.state_args = (char *)g_game.state_args + 1;
	}
	
	if (c != '\0' && keys_held & KEY_SKIP_TEXT) {
		puts((char *) g_game.state_args);
		g_game.state_args = (char *) g_game.state_args +
			strlen((char *) g_game.state_args);
		puts("\n");
		puts("(press Y to continue)");
	}

	return EXIT_SUCCESS;
}





void game_interact(sprite_t *sprite) {
	switch (sprite->interactable_type){
	case INTERACTABLE_TYPE_NONE:
		// ???
		return;
	case INTERACTABLE_TYPE_LORE:
		game_set_state(GAME_STATE_TEXT, (void *) text[sprite->data[0]],
			NULL);
		break;
	case INTERACTABLE_TYPE_DOOR_SWITCH:
		{}
		cell_t *door = map_get_cell(&g_game.map,
			sprite->data[0], sprite->data[1]);
		if (door->tex == CELL_TEX_EMPTY) {
			door->tex = CELL_TEX_DOOR;
		} else {
			door->tex = CELL_TEX_EMPTY;
		}
		break;
	}
}

int game_tick() {
	u64 tick_start = svcGetSystemTick();
	int res;
	switch (g_game.state) {
	case GAME_STATE_PLAYING:
		res = game_tick_play();
		break;
	case GAME_STATE_TEXT:
		res =  game_tick_text();
		break;
	default:
		UNREACHABLE("illegal game state value");
	}
	g_delta = (float) (svcGetSystemTick() - tick_start) / (CPU_TICKS_PER_MSEC * 1000);
	return res;
}


void game_set_state(int state, void *state_args, void (*state_change_callback)()) {
	if (g_game.state_change_callback) g_game.state_change_callback();

	// state exit stuff
	switch (state) {
		
	}
	g_game.state = state;
	g_game.state_args = state_args;
	g_game.state_change_callback = state_change_callback;

	switch (state) {
	case GAME_STATE_TEXT:
		consoleClear();
	}
}
