#ifndef PLAYER_H
#define PLAYER_H

#include <3ds/types.h>
#include "map.h"
#include "maths.h"


#define VEC3_FORWARD (vec3_t) {1, 0, 0}
#define VEC3_RIGHT (vec3_t) {0, 1, 0}
#define VEC3_BACKWARD (vec3_t) {-1, 0, 0}
#define VEC3_LEFT (vec3_t) {0, -1, 0}
// relative to rotation 0 (so facing in +x direction)

#define MOVE_SPEED 2.1
// DEBUG this should be 1
#define ROTATE_SPEED 2.3

#define ITEM_COUNT 3
enum {
	ITEM_CAPTIANS_HAND,
	ITEM_MINE,
	ITEM_LIGHT_SENSOR,
};

typedef struct {
	vec3_t pos;
	float rotation;
	int action;
	u8 items[ITEM_COUNT * 2];
	// saved as {type_0, count_0, type_1, count_1, ...}
} player_t;

enum {
	PLAYER_ACTION_NONE,
	PLAYER_ACTION_INTERACT,
};


// TODO should these all use g_game.player?

void player_init(player_t *player, map_t *map);
void player_tick(player_t *player);

u8 player_get_item_count(player_t *player, u8 item);

void player_handle_input(player_t *player);
void player_move(player_t *player, vec3_t dir);
void player_rotate(player_t *player, float by);


#endif
