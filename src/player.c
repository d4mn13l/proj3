#include "player.h"

#include <3ds.h>
#include <math.h>

#include "map.h"
#include "maths.h"
#include "keymap.h"



void player_init(player_t *player, map_t *map) {
	player->pos = (vec3_t) {map->player_start_x + 0.5,
		map->player_start_y + 0.5, 0.5};
	player->rotation = 0;
	player->action = PLAYER_ACTION_NONE;
}


void player_handle_input(player_t *player, map_t *map, u32 keys, float delta) {
	vec3_t dir = (vec3_t) {0, 0, 0};
	if (keys & KEY_FORWARD) {
		dir = vec3_add(dir, DIR_FORWARD);
	}
	if (keys & KEY_RIGHT) {
		dir = vec3_add(dir, DIR_RIGHT);
	}
	if (keys & KEY_BACK) {
		dir = vec3_add(dir, DIR_BACKWARD);
	}
	if (keys & KEY_LEFT) {
		dir = vec3_add(dir, DIR_LEFT);
	}
	
	if (keys & KEY_TURN_LEFT) {
		player_rotate(player, -ROTATE_SPEED * delta);
	}
	if (keys & KEY_TURN_RIGHT) {
		player_rotate(player, ROTATE_SPEED * delta);
	}

	if (!(dir.x == 0 && dir.y == 0 && dir.z == 0)) {
		dir = vec3_normalised(dir);
		player_move(player, dir, map, delta);
	}
}


void player_move(player_t *player, vec3_t dir, map_t *map, float delta) {
	vec3_t by = vec3_mul_scalar(
			MOVE_SPEED * delta,
			vec3_rotate_y(dir, player->rotation));
	vec3_t new_pos = vec3_add(player->pos, by);

	if (map_is_in_bounds(map, (int) new_pos.x, (int) new_pos.y)
		&& map_get_cell(map, (int) new_pos.x, (int) new_pos.y)->tex
			== CELL_EMPTY)
		{
		player->pos = new_pos;
	}
}


void player_rotate(player_t *player, float by) {
	player->rotation = fmod(player->rotation + by, 2 * PI);
}


