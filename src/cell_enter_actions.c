#include "cell_enter_actions.h"

#include <math.h>


#include "creature.h"
#include "game.h"
#include "map.h"
#include "maths.h"
#include "player.h"
#include "shared.h"



void cea_upstairs_cutscene() {
	// turns the player around and moves the creature into the hallway
	
	static float time = 0.0f;
	static float done_rotating_at = 0.0f;
	static float target_rotation = PI / 2;
	static bool moved_creature = false;

	time += g_delta;
	
	// first rotate the player to face the other way
	if (done_rotating_at == 0.0f) {
		float to_rotate =
			fmodf(g_game.player.rotation - target_rotation, 2 * PI);
		if (to_rotate < 0.0f) to_rotate += 2 * PI;
		// fmod returns a value in range (-2pi, 2pi) but we want
		// to_rotate to be positive in range [0, 2pi)

		if (to_rotate < 0.2f) {
			done_rotating_at = time;
		} else {
			player_rotate(fminf(ROTATE_SPEED * g_delta, to_rotate));
		}
		return;
	}

	// wait a second after rotating

	if (time - done_rotating_at < 1.0f) return;
	
	// move creature into the hallway
	
	if (!moved_creature) {
		creature_move_to((vec3_t) {22.5f, 12.5f, 0.5f});
		// hardcode bc then it might actually work
		moved_creature = true;
	}

	// wait another second
	if (time - done_rotating_at < 2.0f) return;
	
	g_game.player.thinking = "oh shit, gotta run";
	g_game.player.think_for = 3.0f;

	creature_change_action(CREATURE_ACTION_CHASE);
	game_set_state(GAME_STATE_PLAYING, NULL, NULL);
}


void cea_upstairs_chase_trigger(void *vcell, vec3_t cell_pos) {
	static bool done = false;
	if (done) return;
	done = true;

	game_set_state(GAME_STATE_CUTSCENE, (void *) cea_upstairs_cutscene, NULL);
}


void cea_upstairs_chase_spawn(void *vcell, vec3_t cell_pos) {
	// moves the creature closer out of bounds so it starts to blink
	
	// make sure this only happens nnce
	static bool done = false;
	if (done) return;
	done = true;

	vec3_t creature_pos = vec3_add(
	        (vec3_t) {cell_pos.x, cell_pos.y, 0.5}, (vec3_t) {2, 5, 0});

	creature_move_to(creature_pos);
	creature_change_action(CREATURE_ACTION_WAIT);

	g_game.player.thinking = "oh oh. not good.";
	g_game.player.think_for = 2.0f;
}


void cea_upstairs_chase_arm(void *vcell, vec3_t cell_pos) {
	// prepares the chase sequence back to the bridge after picking up the
	// captians hand
	// make sure this only happens nnce
	static bool done = false;

	if (player_get_item_count(ITEM_CAPTIANS_HAND) == 0) {
		// only do this once the captians hand has been picked up
		return;
	}
	
	if (done) return;
	done = true;

	// arm cells above in corrider with spawn and trigger
	cell_t *spawn_cell =
		map_get_cell(&g_game.map, cell_pos.x, cell_pos.y - 3);
	spawn_cell->cell_enter_action = cea_upstairs_chase_spawn;

	cell_t *trigger_cell =
		map_get_cell(&g_game.map, cell_pos.x, cell_pos.y - 7);
	trigger_cell->cell_enter_action = cea_upstairs_chase_trigger;
}
