#include "player.h"

#include <3ds.h>
#include <math.h>

#include "game.h"
#include "map.h"
#include "maths.h"
#include "keymap.h"
#include "renderer.h"
#include "shared.h"


void player_init(player_t *player, map_t *map) {
	player->pos = (vec3_t) {map->player_start_x + CAMERA_HEIGHT,
		map->player_start_y + 0.5, 0.5};
	player->rotation = 0;
	player->action = PLAYER_ACTION_NONE;
}



void player_tick(player_t *player) {
	player_handle_input(player);
}


void player_handle_input(player_t *player) {
	u32 keys_held = hidKeysHeld();
	u32 keys_down = hidKeysDown();

	if (keys_held & KEY_FORWARD) {
		player_move(player, VEC3_FORWARD);
	}
	if (keys_held & KEY_RIGHT) {
		player_move(player, VEC3_RIGHT);
	}
	if (keys_held & KEY_BACK) {
		player_move(player, VEC3_BACKWARD);
	}
	if (keys_held & KEY_LEFT) {
		player_move(player, VEC3_LEFT);
	}
	// moving seperately for every direction makes sure that you can
	// move diagonally into a wall and slide along, instead of getting
	// stuck because its just one movement that results in an invalid
	// position
	
	if (keys_held & KEY_TURN_LEFT) {
		player_rotate(player, -ROTATE_SPEED * g_delta);
	}
	if (keys_held & KEY_TURN_RIGHT) {
		player_rotate(player, ROTATE_SPEED * g_delta);
	}

	if (keys_down & KEY_INTERACT) {
		prop_t *interactable =
			map_get_interactable(&g_game.map,
				player->pos.x, player->pos.y);
		if (interactable != NULL) {
			game_interact(interactable);
		}
	}
}


void player_move(player_t *player, vec3_t dir) {
	vec3_t by = vec3_mul_scalar(
			MOVE_SPEED * g_delta,
			vec3_rotate_y(dir, player->rotation));
	vec3_t new_pos = vec3_add(player->pos, by);

	if (map_is_in_bounds(&g_game.map, (int) new_pos.x, (int) new_pos.y)
			&& map_get_cell(&g_game.map, (int) new_pos.x,
			(int) new_pos.y)->tex == CELL_TEX_EMPTY) {
		player->pos = new_pos;
	}
}


void player_rotate(player_t *player, float by) {
	player->rotation = fmod(player->rotation + by, 2 * PI);
}


