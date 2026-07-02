#include "creature.h"
#include "maths.h"
#include "renderer.h"
#include "shared.h"



void creature_init(creature_t *creature, float x, float y) {
	creature->pos = (vec3_t) {x, y, CAMERA_HEIGHT};
	creature->los_player = false;
	creature->action = CREATURE_ACTION_ROAM;
}


bool creature_has_los(creature_t *creature, vec3_t target) {
	// compare distance to target and distance to first wall in target
	// direction
	// wall behind target -> los
	// target behind wall -> no los
	vec3_t line_to_target = vec3_sub(g_game.player.pos, creature->pos);
	float distance = vec3_length(line_to_target);
	
	ray_cast_result_t rc_res = cast_ray(&g_game.map, creature->pos,
		vec3_mul_scalar(1 / distance, line_to_target));

	return vec3_length(vec3_sub(rc_res.pos, creature->pos)) > distance;
}


void creature_update_los_player(creature_t *creature) {
	creature->los_player = creature_has_los(creature, g_game.player.pos);
	if (creature->los_player) {
		creature->last_spotted_pos = g_game.player.pos;
	}
}


void creature_tick_chase(creature_t *creature) {
	creature_update_los_player(creature);
	// check for LOS to last_spotted_pos
	if (!creature_has_los(creature, creature->last_spotted_pos)) {
		puts("no los to lsp");
		creature_change_action(creature, CREATURE_ACTION_ROAM);
		return;
	}

	// check if lsp has been reached
	if ((int) creature->pos.x == (int) creature->last_spotted_pos.x
			&& (int) creature->pos.y
			== (int) creature->last_spotted_pos.y) {
		// TODO
		creature_change_action(creature, CREATURE_ACTION_ROAM);
		return;
	}

	// move towards lsp

	vec3_t dir = vec3_normalised(
	        vec3_sub(creature->last_spotted_pos, creature->pos));

	creature->pos = vec3_add(creature->pos,
	        vec3_mul_scalar(CREATURE_CHASE_SPEED * g_delta, dir));
	
}


void creature_change_action(creature_t *creature, int to) {
	creature->action = to;
}


void creature_tick(creature_t *creature) {
	switch (creature->action) {
	case CREATURE_ACTION_ROAM:
		creature_update_los_player(creature);
		if (creature->los_player) {
			creature->action = CREATURE_ACTION_CHASE;
			break;
		}

		// TODO
		break;

	case CREATURE_ACTION_CHASE:
		creature_tick_chase(creature);
		break;


	}
}
