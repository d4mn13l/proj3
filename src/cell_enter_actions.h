#ifndef CELL_ENTER_ACTIONS_H
#define CELL_ENTER_ACTIONS_H


#include "map.h"
#include "maths.h"


void cea_upstairs_chase_arm(void *vcell, vec3_t cell_pos);
void cea_upstairs_chase_spawn(void *vcell, vec3_t cell_pos);
void cea_upstairs_chase_trigger(void *vcell, vec3_t cell_pos);


typedef union {
	char *name;
	cell_enter_action_f action;
} cea_pair;

static const cea_pair cell_enter_actions[] = {
	{.name = "arm_upstairs_chase"}, {.action = cea_upstairs_chase_arm},
};


#endif
