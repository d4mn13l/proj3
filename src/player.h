#ifndef PLAYER_H
#define PLAYER_H

#include <3ds/types.h>
#include "map.h"
#include "maths.h"


#define DIR_FORWARD (vec3_t) {1, 0, 0}
#define DIR_RIGHT (vec3_t) {0, 1, 0}
#define DIR_BACKWARD (vec3_t) {-1, 0, 0}
#define DIR_LEFT (vec3_t) {0, -1, 0}
// relative to rotation 0 (so facing in +x direction)

#define MOVE_SPEED 1
#define ROTATE_SPEED 1.5

typedef struct {
	vec3_t pos;
	float rotation;
	int action;
} player_t;

enum {
	PLAYER_ACTION_NONE,
	PLAYER_ACTION_INTERACT,
};


void player_init(player_t *player, map_t *map);
void player_handle_input(player_t *player, map_t *map, u32 keys_held,
	u32 keys_pressed, float delta);
void player_move(player_t *player, vec3_t dir, map_t *map, float delta);
void player_rotate(player_t *player, float by);


#endif
