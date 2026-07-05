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

#define MOVE_SPEED 1
#define ROTATE_SPEED 2.3

enum {
	ITEM_CAPTIANS_HAND = 0,
	ITEM_MINE = 1,
	ITEM_LIGHT_SENSOR = 2,

	ITEM_COUNT = 3,
};

static const char ITEM_NAMES[ITEM_COUNT][16] = {
	"captain's hand", "mine", "light sensor"
};

typedef struct {
	vec3_t pos;
	float rotation;
	int action;
	u8 items[ITEM_COUNT];
	char *thinking;
	// displayed on the bottom screen for think_for s
	float think_for;
} player_t;

enum {
	PLAYER_ACTION_NONE,
	PLAYER_ACTION_INTERACT,
};


// TODO should these all use g_game.player?

void player_init(map_t *map);
void player_tick();

u8 player_get_item_count(u8 item);
void player_pickup_item(u8 item);


void player_handle_input();
void player_move(vec3_t dir);
void player_rotate(float by);


#endif
