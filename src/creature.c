#include "creature.h"
#include "maths.h"
#include "renderer.h"
#include "shared.h"



void creature_init(creature_t *creature, float x, float y) {
	creature->pos = (vec3_t) {x, y, CAMERA_HEIGHT};
	creature->has_los = false;
	creature->action = CREATURE_ACTION_WANDER;
}


void creature_update_los(creature_t *creature) {
	// compare distance to player and distance to first wall in player
	// direction
	// wall behind player -> los
	// player behind wall -> no los
	vec3_t line_to_player = vec3_sub(g_game.player->pos, creature->pos);
	float player_distance = vec3_length(line_to_player);
	
	ray_cast_result_t rc_res = cast_ray(g_game.map, creature->pos,
		vec3_mul_scalar(1/player_distance, line_to_player));

	if (vec3_length(vec3_sub(rc_res.pos, creature->pos)) > player_distance) {
		creature->has_los = true;
		creature->last_spotted_pos = g_game.player->pos;
	}
}

void creature_tick(creature_t *creature) {
	switch (creature->action) {
	case CREATURE_ACTION_WANDER:
		creature_update_los(creature);
		if (creature->has_los) {
			creature->action = CREATURE_ACTION_CHASE;
			break;
		}

		// TODO
		break;
		
	}
}
