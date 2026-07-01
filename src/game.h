#ifndef GAME_H
#define GAME_H

#include "map.h"
#include "player.h"
#include "renderer.h"


#define LORE_CHAR_DELAY_MS 69
#define LORE_NEWLINE_DELAY_MS 420
// how many ms elapse between chars when printing them in text game state

enum {
	GAME_STATE_PLAYING,
	GAME_STATE_TEXT,
		// expects pointer to next char to print in state_args
};


typedef struct {
	map_t *map;
	player_t *player;
	tex_atlas_t *ta;
	int state;
	void *state_args;
} game_t;



// use g_game global from shared.h

int game_tick();

void game_set_state(int state, void *state_args);

void game_interact(prop_t *prop);

#endif
