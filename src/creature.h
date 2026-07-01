#ifndef CREATURE_H
#define CREATURE_H


#include "maths.h"

typedef struct {
	vec3_t pos;
	bool has_los;
	vec3_t last_spotted_player_pos;
} creature_t;

// line of sight check
void creature_update(creature_t *creature);

void creature_tick(creature_t *creature);


#endif
