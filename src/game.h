#ifndef GAME_H
#define GAME_H

#include "creature.h"
#include "map.h"
#include "player.h"
#include "renderer.h"


#define LORE_CHAR_DELAY_MS 69
#define LORE_NEWLINE_DELAY_MS 420
// how many ms elapse between chars when printing them in text game state

#define BLINK_INTERVAL_MS 1000
// how long the whole blink takes (on + off period)
#define BLINK_MAX_DISTANCE 10
// how close the creature must be for blinking to happen

enum {
	GAME_STATE_PLAYING,
	GAME_STATE_TEXT,
		// expects pointer to next char to print in state_args
	GAME_STATE_JUMPSCARE,
};


typedef struct {
	map_t map;
	player_t player;
	creature_t creature;
	tex_atlas_t ta;
	int state;
	void *state_args;
	void (*state_change_callback)();
} game_t;



// use g_game global from shared.h
void game_init();
int game_tick();
void game_deinit();

void game_set_state(int state, void *state_args, void (*state_change_callback)());

void game_interact(sprite_t *sprite);

#endif
