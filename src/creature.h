#ifndef CREATURE_H
#define CREATURE_H


#include "maths.h"


#define CREATURE_ROAM_SPEED 0.5
#define CREATURE_CHASE_SPEED 1.2


// TODO should this be done with function pointers to the tick functions?
enum {
	CREATURE_ACTION_ROAM,
	CREATURE_ACTION_CHASE,
};


enum {
	DIR_FORWARD = 0,
	DIR_RIGHT = 1,
	DIR_BACK = 2,
	DIR_LEFT = 3,
};


typedef struct {
	vec3_t pos;
	int facing; // DIR_...
	int action;
	bool los_player;
	vec3_t last_spotted_pos;
} creature_t;

// line of sight check
void creature_update_los_player(creature_t *creature);
void creature_change_action(creature_t *creature, int to);

void creature_tick(creature_t *creature);


#endif
