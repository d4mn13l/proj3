#include "creature.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "map.h"
#include "maths.h"
#include "renderer.h"
#include "shared.h"
#include "util.h"



void creature_init(map_t *map) {
	g_game.creature.pos = map->creature_start_pos;
	g_game.creature.los_player = false;

	g_game.creature.sprite.pos = g_game.creature.pos;
	g_game.creature.sprite.interactable_type = INTERACTABLE_TYPE_NONE;
	g_game.creature.sprite.width = 0.5;
	g_game.creature.sprite.tex = 7 + 1 * g_game.ta.nx;
	g_game.creature.current_roam_path = 'c';
	// creature tex is at (7/1)
	memset(&g_game.creature.sprite.data, 0,
		sizeof(g_game.creature.sprite.data));
	if (map_is_in_bounds(&g_game.map, map->creature_start_pos.x,
	        	map->creature_start_pos.y))
		map_get_cell(&g_game.map, map->creature_start_pos.x,
		        	map->creature_start_pos.y)->sprites[0]
			= g_game.creature.sprite;
	// beautiful indentation

	creature_change_action(CREATURE_ACTION_ROAM);
}


bool creature_has_los(vec3_t target) {
	// compare distance to target and distance to first wall in target
	// direction
	// wall behind target -> los
	// target behind wall -> no los
	vec3_t line_to_target = vec3_sub(target, g_game.creature.pos);
	line_to_target.z = 0;
	float distance = vec3_length(line_to_target);
	
	ray_cast_result_t rc_res = cast_ray(&g_game.map, g_game.creature.pos,
		vec3_mul_scalar(1 / distance, line_to_target));

	return vec3_length(vec3_sub(rc_res.pos, g_game.creature.pos)) > distance;
}


void creature_update_los_player() {
	g_game.creature.los_player = creature_has_los(g_game.player.pos);
	if (g_game.creature.los_player) {
		creature_update_move_target(g_game.player.pos);
	}
}


void creature_update_move_target(vec3_t to) {
	fprintf(g_log_file, "updating move target to %f %f\n", to.x, to.y);
	g_game.creature.move_target = (vec3_t) {to.x, to.y, 0.5};
}


u8 creature_get_path_options(vec3_t out[4],
		bool include_last_cell, bool include_vents) {
	// check all orthogonally adjacent cells to the creature
	// puts centered absolute cell coords in out and returns how many path
	// options were found
	size_t cx = g_game.creature.pos.x;
	size_t cy = g_game.creature.pos.y;
	size_t lcx = g_game.creature.last_cell.x;
	size_t lcy = g_game.creature.last_cell.y;
	map_t *map = &g_game.map;
	u8 dir_count = 0;
	// if dir_count == 2, this contains the cell that the creature will
	// move towards
	fprintf(g_log_file, "getting path options at %f %f, last cell %f %f\n", g_game.creature.pos.x, g_game.creature.pos.y, g_game.creature.last_cell.x, g_game.creature.last_cell.y);
	if (map_is_in_bounds(map, cx + 1, cy)) {
		int tex = map_get_cell(map, cx + 1, cy)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx + 1 != lcx || cy != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx + 1.5, cy + 0.5, 0.5};
			dir_count++;
		}
	}
	if (map_is_in_bounds(map, cx, cy + 1)) {
		int tex = map_get_cell(map, cx, cy + 1)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx != lcx || cy + 1 != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx + 0.5, cy + 1.5, 0.5};
			dir_count++;
		}
	}
	if (map_is_in_bounds(map, cx - 1, cy)) {
		int tex = map_get_cell(map, cx - 1, cy)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx - 1 != lcx || cy != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx - 0.5, cy + 0.5, 0.5};
			dir_count++;
		}
	}
	if (map_is_in_bounds(map, cx, cy - 1)) {
		int tex = map_get_cell(map, cx, cy - 1)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx != lcx || cy - 1 != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx + 0.5, cy - 0.5, 0.5};
			dir_count++;
		}
	}
	fprintf(g_log_file, "got %d paths\n", dir_count);
	return dir_count;
}



void creature_tick_chase() {
	// check if chase target hasnt been reached
	if (!vec3_equal_approx(g_game.creature.pos, g_game.creature.move_target)) {
		if (!creature_has_los(g_game.creature.move_target)) {
			fputs("no los -> roam\n", g_log_file);
			// can happen when ie a door is closed
			// important to check that pos != chase_target,
			// otherwise the LOS checking failes r.z == 0 assert
			// in cast_ray
			creature_change_action(CREATURE_ACTION_ROAM);
			return;
		}

		// move towards move_target
		// when in the same cell, snap to it to avoid overshooting
		creature_update_los_player();
		if ((int) g_game.creature.pos.x
					== (int) g_game.creature.move_target.x
				&& (int) g_game.creature.pos.y
				== (int) g_game.creature.move_target.y) {
			creature_move_to(g_game.creature.move_target);
		} else {
			creature_move_towards_target(CREATURE_CHASE_SPEED);
		}
		return;
	}
		
	// check if there is just 1 possible path to continue through
	// the map (in the case of a corner). in that case, chase
	// in that direction. otherwise, roamä

	fprintf(g_log_file, "reached chase target\n");
	
	vec3_t paths[4];
	u8 paths_count = creature_get_path_options(paths, false, false);
	fprintf(g_log_file, "%d", paths_count);
	if (paths_count != 1) {
		// more / less than 1 option to chase the player, so continue
		// roaming
		// dont include vents here because the creature knows the
		// player cant use them
		// TODO after losing the player, continue chasing a few times
		// in random directions
		creature_change_action(CREATURE_ACTION_ROAM);
		fputs("changed to roaming\n", g_log_file);
		return;
	}
	
	fprintf(g_log_file, "continuing chasing toward %f %f\n", paths[0].x, paths[0].y);
	creature_update_move_target(
		(vec3_t) {paths[0].x + 0.5, paths[0].y + 0.5, 0.5});
	
}


void creature_tick_roam() {
	fflush(g_log_file);
	// DEBUG
	creature_update_los_player();
	if (g_game.creature.los_player) {
		creature_change_action(CREATURE_ACTION_CHASE);
		return;
	}

	// for now ignore roam paths. instead, at every intersection, randomly
	// choose a path, but choose the one towards the player with a higher
	// probability
	
	if (!is_same_cell(g_game.creature.pos, g_game.creature.move_target)) {
		creature_move_towards_target(CREATURE_ROAM_SPEED);
		return;
	}

	creature_choose_roam_target();
}


void creature_choose_roam_target() {
	// creature moved to target cell, so we have to select a new one
	vec3_t paths[4];
	size_t paths_count = creature_get_path_options(paths, false, true);
	fprintf(g_log_file, "at %f %f got %d paths\n", g_game.creature.pos.x, g_game.creature.pos.y, paths_count);
	if (paths_count == 0) {
		g_game.creature.move_target = g_game.creature.last_cell;
		fprintf(g_log_file, "cul-de-sac -> moving back\n");
		return;
	}

	// first randomly determine whether the creature should go towards the
	// player or pick a random path
	if ((float) rand() / RAND_MAX < CREATURE_ROAM_CHOOSE_RANDOM_PATH_PROBABLILITY) {
		creature_update_move_target(paths[rand() % paths_count]);
		fprintf(g_log_file, "picked %f %f randomly\n", g_game.creature.move_target.x, g_game.creature.move_target.y);
		return;
	}

	// now choose the direction for which |dir - normalised(line_to_player)|
	// is the smallest
	// we can use vec3_length_squared instead of vec3_length
	// this should determine which direction faces the closest to the player

	float smallest_distance = HUGE_VALF;
	int smallest_index = -1;
	vec3_t player_direction = vec3_normalised(
		vec3_sub(g_game.player.pos, g_game.creature.pos));
	vec3_t centered_creature_pos = center_in_cell(g_game.creature.pos);
	for (size_t i = 0; i < paths_count; i++) {
		float d = vec3_length_squared(vec3_sub(
			vec3_sub(paths[i], centered_creature_pos),
				// direction, already normalised
			player_direction));
		if (d <  smallest_distance) {
			smallest_distance = d;
			smallest_index = i;
		}
	}
	ASSERT(smallest_index != -1);
	fprintf(g_log_file, "picked %d %d (closest to player\n)", (int) g_game.creature.move_target.x, (int) g_game.creature.move_target.y);

	creature_update_move_target(paths[smallest_index]);
}



void creature_move(vec3_t by) {
	size_t ocx = g_game.creature.pos.x;
	size_t ocy = g_game.creature.pos.y;
	// old cell coordinates

	g_game.creature.pos = vec3_add(g_game.creature.pos, by);
	g_game.creature.pos.z = 0.5;
	g_game.creature.sprite.pos = g_game.creature.pos;
	
	if (ocx != (int) g_game.creature.pos.x
			|| ocy != (int) g_game.creature.pos.y) {
		// fprintf(g_log_file, "moved to new cell oc: %d %d, pos: %f %f", ocx, ocy, g_game.creature.pos.x, g_game.creature.pos.y);
		// creature moved to another cell
		if ((size_t) g_game.creature.pos.x != ocx) {
			g_game.creature.last_cell.x = ocx;
			g_game.creature.last_cell.y = g_game.creature.pos.y;
		} else {
			g_game.creature.last_cell.y = ocy;
			g_game.creature.last_cell.x = g_game.creature.pos.x;
		}
		// ^ this makes sure that last_cell is always orthgonally
		// adjacent to the cell of pos
		// if we just assigned ocx and ocy to last_cell.[x,y],
		// that would not be the case if the creature moved diagonally
		// to the next cell
		// also this assumes the game is running fast enough so that
		// the creature doesnt skip cells
		
		if (map_is_in_bounds(&g_game.map, ocx, ocy))
			map_get_cell(&g_game.map, ocx, ocy)->sprites[0]
				= SPRITE_EMPTY;
	}

	size_t ncx = g_game.creature.pos.x;
	size_t ncy = g_game.creature.pos.y;


	if (map_is_in_bounds(&g_game.map, ncx, ncy)) {
		// update creature sprite
		cell_t *new_cell = map_get_cell(&g_game.map, ncx, ncy);
		new_cell->sprites[0]
			 = g_game.creature.sprite;

		if ((ncx != ocx || ncy != ocy) &&
				new_cell->flags & CELL_FLAG_MINED) {
			creature_change_action(CREATURE_ACTION_STUNNED);
			map_remove_sprite(&g_game.map, ncx, ncy, SPRITE_TEX_MINE);
		}
	}
}


void creature_move_to(vec3_t to) {
	creature_move(vec3_sub(to, g_game.creature.pos));
}


void creature_move_towards_target(float speed) {
	vec3_t dir = vec3_normalised(vec3_sub(g_game.creature.move_target,
		g_game.creature.pos));
	creature_move(vec3_mul_scalar(speed * g_delta, dir));
}


void creature_change_action(int to) {
	fprintf(g_log_file, "changing creature action to %d\n", to);
	g_game.creature.action = to;
	switch (to) {
	break; case CREATURE_ACTION_STUNNED:
		g_game.creature.stunned_timer = CREATURE_STUNNED_TIME;
	break; case CREATURE_ACTION_ROAM:
		creature_choose_roam_target();
	}
}


void creature_tick() {
	if (is_same_cell(g_game.creature.pos, g_game.player.pos)) {
		game_set_state(GAME_STATE_JUMPSCARE, NULL, NULL);
		return;
	}
	switch (g_game.creature.action) {
	break; case CREATURE_ACTION_ROAM:
		creature_tick_roam();

	break; case CREATURE_ACTION_CHASE:
		creature_tick_chase();
	
	break; case CREATURE_ACTION_WAIT:

	break; case CREATURE_ACTION_STUNNED:
		g_game.creature.stunned_timer -= g_delta;
		if (g_game.creature.stunned_timer < 0)
			creature_change_action(CREATURE_ACTION_ROAM);
	break; default:
		UNREACHABLE("illegal creature state");
	}
}
