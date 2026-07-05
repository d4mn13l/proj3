#ifndef CREATURE_H
#define CREATURE_H


#include "map.h"
#include "maths.h"


#define CREATURE_ROAM_SPEED 0.5
#define CREATURE_CHASE_SPEED 1

#define CREATURE_STUNNED_TIME 2

enum {
	CREATURE_ACTION_ROAM,
	CREATURE_ACTION_CHASE,
	CREATURE_ACTION_STUNNED,
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
	float stunned_timer;
	bool los_player;
	vec3_t chase_target;
	sprite_t sprite;
} creature_t;

void creature_init(float x, float y);

// line of sight check
void creature_update_los_player();
void creature_update_chase_target(vec3_t to);
void creature_change_action(int to);

void creature_move(vec3_t by);
void creature_move_to(vec3_t to);
void creature_tick();


#endif
