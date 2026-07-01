#ifndef CREATURE_H
#define CREATURE_H


#include "maths.h"


#define CREATURE_WANDER_SPEED 0.5
#define CREATURE_CHASE_SPEED 1.2


enum {
	CREATURE_ACTION_WANDER,
	CREATURE_ACTION_CHASE,
};

typedef struct {
	vec3_t pos;
	int action;
	bool has_los;
	vec3_t last_spotted_pos;
} creature_t;

// line of sight check
void creature_update_los(creature_t *creature);

void creature_tick(creature_t *creature);


#endif
