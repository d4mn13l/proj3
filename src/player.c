#include "player.h"

#include <3ds.h>
#include <math.h>
#include <string.h>

#include "game.h"
#include "map.h"
#include "maths.h"
#include "keymap.h"
#include "renderer.h"
#include "shared.h"
#include "util.h"


void player_init(map_t *map) {
	g_game.player.pos = (vec3_t) {map->player_start_x + CAMERA_HEIGHT,
		map->player_start_y + 0.5, 0.5};
	g_game.player.rotation = 0;
	memset(&g_game.player.items, 0, sizeof(g_game.player.items));
	g_game.player.held_item = ITEM_COUNT;
	g_game.player.thinking = NULL;
	g_game.player.think_for = 0.0f;
	g_game.player.items[ITEM_MINE] = 2;
}



void player_tick() {
	player_handle_input();
	if (g_game.player.thinking != NULL) {
		g_game.player.think_for -= g_delta;
		if (g_game.player.think_for < 0)
			g_game.player.thinking = NULL;
	}
}


void player_handle_input() {
	u32 keys_held = hidKeysHeld();
	u32 keys_down = hidKeysDown();

	vec3_t dir = (vec3_t) {0, 0, 0};

	if (keys_held & KEY_FORWARD) {
		dir = vec3_add(dir, VEC3_FORWARD);
	}
	if (keys_held & KEY_RIGHT) {
		dir = vec3_add(dir, VEC3_RIGHT);
	}
	if (keys_held & KEY_BACK) {
		dir = vec3_add(dir, VEC3_BACKWARD);
	}
	if (keys_held & KEY_LEFT) {
		dir = vec3_add(dir, VEC3_LEFT);
	}

	player_move(dir);
	
	if (keys_held & KEY_TURN_LEFT) {
		player_rotate(-ROTATE_SPEED * g_delta);
	}
	if (keys_held & KEY_TURN_RIGHT) {
		player_rotate(ROTATE_SPEED * g_delta);
	}

	if (keys_down & KEY_INTERACT) {
		sprite_t *interactable =
			map_get_interactable(&g_game.map,
				g_game.player.pos.x, g_game.player.pos.y);
		if (interactable != NULL) {
			game_interact(interactable);
		}
	}

	if (keys_down & KEY_CYCLE_ITEM) {
		player_cycle_held_item();
	}

	if (keys_down & KEY_USE_ITEM) {
		player_use_held_item();
	}
}


u8 player_get_item_count(u8 item) {
	ASSERT(item < ITEM_COUNT);
	return g_game.player.items[item];
}

void player_pickup_item(u8 item) {
	ASSERT(item < ITEM_COUNT);
	g_game.player.items[item]++;
}


void player_use_held_item() {
	switch (g_game.player.held_item) {
	case ITEM_CAPTIANS_HAND:
	case ITEM_VALUABLE_CORPORATE_PROPERTY:
		g_game.player.thinking = "i cant use that";
		g_game.player.think_for = 2.0f;
		break;
	case ITEM_MINE:
		if (map_add_sprite(&g_game.map, g_game.player.pos.x,
				g_game.player.pos.y, SPRITE_TEX_MINE)) {
			g_game.player.thinking = "cant put it here (dont ask)";
			g_game.player.think_for = 3.0f;
			return;
		}
		map_get_cell(&g_game.map, g_game.player.pos.x,
			g_game.player.pos.y)->flags |= CELL_FLAG_MINED;
		g_game.player.items[ITEM_MINE]--;
		if (g_game.player.items[ITEM_MINE] == 0)
			player_cycle_held_item();
		break;
	};
}


void player_cycle_held_item() {
	u8 start_item = g_game.player.held_item;
	// go to next item until we find one whose count is not zero
	// or we do one full loop
	do {
		g_game.player.held_item =
			(g_game.player.held_item + 1) % (ITEM_COUNT + 1);
	} while (g_game.player.items[g_game.player.held_item] == 0 &&
			g_game.player.held_item != start_item);
}


void player_move(vec3_t dir) {
	vec3_t by = vec3_mul_scalar(
			MOVE_SPEED * g_delta,
			vec3_rotate_y(dir, g_game.player.rotation));

	// move in x and y seperately to avoid getting stuck when walking into
	// wall at an angle
	vec3_t new_pos = g_game.player.pos;

	if (map_is_in_bounds(&g_game.map, (size_t) (new_pos.x + by.x),
				(size_t) new_pos.y)
			&& map_get_cell(&g_game.map, (size_t) (new_pos.x + by.x),
			(size_t) new_pos.y)->tex == CELL_TEX_EMPTY) {
		new_pos.x += by.x;
	}
	if (map_is_in_bounds(&g_game.map, (size_t) new_pos.x,
				(size_t) (new_pos.y + by.y))
			&& map_get_cell(&g_game.map, (size_t) new_pos.x,
			(size_t) (new_pos.y + by.y))->tex == CELL_TEX_EMPTY) {
		new_pos.y += by.y;
	}

	if (!is_same_cell(g_game.player.pos, new_pos)) {
		map_player_entered_cell(&g_game.map, new_pos);
	}
	g_game.player.pos = new_pos;
}


void player_rotate(float by) {
	g_game.player.rotation = fmodf(g_game.player.rotation + by, 2 * PI);
}


