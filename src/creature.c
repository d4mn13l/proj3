#include "creature.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "map.h"
#include "maths.h"
#include "renderer.h"
#include "shared.h"
#include "util.h"



void creature_init(float x, float y) {
	g_game.creature.pos = (vec3_t) {x, y, CAMERA_HEIGHT};
	g_game.creature.los_player = false;
	g_game.creature.action = CREATURE_ACTION_ROAM;

	g_game.creature.sprite.pos = g_game.creature.pos;
	g_game.creature.sprite.interactable_type = INTERACTABLE_TYPE_NONE;
	g_game.creature.sprite.width = 0.5;
	g_game.creature.sprite.tex = 7 + 1 * g_game.ta.nx;
	// creature tex is at (7/1)
	memset(&g_game.creature.sprite.data, 0,
		sizeof(g_game.creature.sprite.data));
	if (map_is_in_bounds(&g_game.map, x, y))
		map_get_cell(&g_game.map, x, y)->sprites[0]
			= g_game.creature.sprite;
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
		creature_update_chase_target(g_game.player.pos);
	}
}


void creature_update_chase_target(vec3_t to) {
	// creature->chase_target =
		// (vec3_t) {floorf(to.x) + 0.5, floorf(to.y) + 0.5, 0.5};
	g_game.creature.chase_target = (vec3_t) {to.x, to.y, 0.5};
}


u8 creature_get_path_options(vec3_t out[4],
		bool include_last_cell, bool include_vents) {
	// check all orthogonally adjacent cells to the creature
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
			out[dir_count] = (vec3_t) {cx + 1, cy, 0.5};
			dir_count++;
		}
	}
	if (map_is_in_bounds(map, cx, cy + 1)) {
		int tex = map_get_cell(map, cx, cy + 1)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx != lcx || cy + 1 != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx, cy + 1, 0.5};
			dir_count++;
		}
	}
	if (map_is_in_bounds(map, cx - 1, cy)) {
		int tex = map_get_cell(map, cx - 1, cy)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx - 1 != lcx || cy != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx - 1, cy, 0.5};
			dir_count++;
		}
	}
	if (map_is_in_bounds(map, cx, cy - 1)) {
		int tex = map_get_cell(map, cx, cy - 1)->tex;
		bool tex_valid = tex == CELL_TEX_EMPTY ||
			(include_vents && tex == CELL_TEX_VENT);
		bool pos_valid = include_last_cell || cx != lcx || cy - 1 != lcy;
		if (tex_valid && pos_valid) {
			out[dir_count] = (vec3_t) {cx, cy - 1, 0.5};
			dir_count++;
		}
	}
	fprintf(g_log_file, "got %d paths\n", dir_count);
	return dir_count;
}



void creature_tick_chase() {
	// check if chase target hasnt been reached
	if (!vec3_equal_approx(g_game.creature.pos, g_game.creature.chase_target)) {
		if (!creature_has_los(g_game.creature.chase_target)) {
			fputs("no los -> roam\n", g_log_file);
			// can happen when ie a door is closed
			// important to check that pos != chase_target,
			// otherwise the LOS checking failes r.z == 0 assert
			// in cast_ray
			creature_change_action(CREATURE_ACTION_ROAM);
			return;
		}

		// move towards lsp
		// slow down proportionally to the distance to chase_target
		// when in the same cell to make sure that it is not
		// overshot 
		creature_update_los_player();
		float speed = CREATURE_CHASE_SPEED * g_delta;
		if ((int) g_game.creature.pos.x
					== (int) g_game.creature.chase_target.x
				&& (int) g_game.creature.pos.y
				== (int) g_game.creature.chase_target.y) {
			speed = vec3_length(vec3_sub(g_game.creature.pos,
			        g_game.creature.chase_target));
		}
		vec3_t dir = vec3_normalised(
			vec3_sub(g_game.creature.chase_target,
				g_game.creature.pos));
		creature_move(vec3_mul_scalar(speed, dir));
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
	creature_update_chase_target(
		(vec3_t) {paths[0].x + 0.5, paths[0].y + 0.5, 0.5});
	
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


void creature_change_action(int to) {
	fprintf(g_log_file, "changing creature action to %d\n", to);
	g_game.creature.action = to;
	switch (to) {
	case CREATURE_ACTION_STUNNED:
		g_game.creature.stunned_timer = CREATURE_STUNNED_TIME;
	}
}


void creature_tick() {
	switch (g_game.creature.action) {
	case CREATURE_ACTION_ROAM:
		creature_update_los_player();
		if (g_game.creature.los_player) {
			g_game.creature.action = CREATURE_ACTION_CHASE;
			break;
		}

		// TODO
		break;

	case CREATURE_ACTION_CHASE:
		creature_tick_chase();
		break;
	
	case CREATURE_ACTION_WAIT:
		break;

	case CREATURE_ACTION_STUNNED:
		g_game.creature.stunned_timer -= g_delta;
		if (g_game.creature.stunned_timer < 0)
			creature_change_action(CREATURE_ACTION_ROAM);
		break;
	default:
		UNREACHABLE("illegal creature state");
	}
}
