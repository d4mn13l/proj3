#ifndef CREATURE_H
#define CREATURE_H


#include "map.h"
#include "maths.h"


#define CREATURE_ROAM_SPEED 0.5
#define CREATURE_CHASE_SPEED 1.1


// TODO should this be done with function pointers to the tick functions?
enum {
	CREATURE_ACTION_ROAM,
	CREATURE_ACTION_CHASE,
	CREATURE_ACTION_WAIT,
};


enum {
	DIR_FORWARD = 0,
	DIR_RIGHT = 1,
	DIR_BACK = 2,
	DIR_LEFT = 3,
};


typedef struct {
	vec3_t pos;
	vec3_t last_cell;
	// this is always orthogonally adjacent to the cell of pos
	int action;
	bool los_player;
	vec3_t chase_target;
	sprite_t sprite;
} creature_t;

void creature_init(creature_t *creature, float x, float y);

// line of sight check
void creature_update_los_player(creature_t *creature);
void creature_update_chase_target(creature_t *creature, vec3_t to);
void creature_change_action(creature_t *creature, int to);

void creature_move(creature_t *creature, vec3_t by);
void creature_move_to(creature_t *creature, vec3_t to);
void creature_tick(creature_t *creature);


#endif
