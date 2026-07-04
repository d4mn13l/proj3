#ifndef MAP_H
#define MAP_H

#include <3ds.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "maths.h"


#define MAX_MAP_DIMENSIONS 255
// make sure the dimensions fit into an 8 bit value

#define CELL_CHAR_EMPTY ' '
#define CELL_CHAR_PLAYER_START 'S'

#define MAP_SECTION_SEPERATOR ']'
#define SPRITE_DATA_COUNT 3

#define MAX_SPRITES_PER_CELL 4
// including 1 reserved slot for the creature



typedef int8_t tex_t;

enum {
	CELL_TEX_EMPTY = -1,
	// indeces from 0 upwards are used for textured walls
	CELL_TEX_DEFAULT = 2,
	// except for this one, walls with non-digit symbols should be treated
	// as having tex index 2
	CELL_TEX_DOOR = 8,
	CELL_TEX_VENT = 10,
	// TODO make vent
};

enum {
	CELL_FLAGS_NONE,
	CELL_FLAG_SHADE_DEFAULT,
	CELL_FLAG_SHADE_BLINK,
};

enum {
	INTERACTABLE_TYPE_NONE = 0,
	INTERACTABLE_TYPE_LORE = 1,
		// index of lore string in data[0]
	INTERACTABLE_TYPE_PICKUP = 2,
	INTERACTABLE_TYPE_DOOR_SWITCH = 3,
		// expects cell coordinates of the door in data[0] (x) and
		// data[1] (y)
		// TODO: make data[3] be an item requirement
};



typedef struct {
	vec3_t pos;
	// z unused but must be set to 0.5
	float width;
	// from centre to side
	tex_t tex;
	
	// ignore these when this is not interactable:
	// also note that a cell can only have 1 interactable
	u8 interactable_type;
	u8 data[SPRITE_DATA_COUNT];
} sprite_t;
// encoding in map.txt:
// sprite: x, y, tex_x, tex_y
// interactable: type, x, y, tex_x, tex_y, data[0], .., data[n]


#define SPRITE_EMPTY (sprite_t) { (vec3_t) {0,0,0}, 0, 0, 0, {0}}


typedef struct {
	tex_t tex;
	sprite_t sprites[MAX_SPRITES_PER_CELL];
	// the first sprite slot is reserved for the creature
	// reserving 4 sprite slots is a huge waste of memory but its not
	// tight so whatever
	int flags;
	u8 roam_path;
	// 0 -> not on any path
} cell_t;


typedef struct {
	size_t w, h;
	size_t player_start_x, player_start_y;
	cell_t *cells;
} map_t;



void map_load(map_t *map, FILE *map_file);
void map_free(map_t *map);

// returns EXIT_FAILURE if there were no more free sprite slots, otherwise
// returns EXIT_SUCCESS
int map_add_sprite(map_t *map, float x, float y, tex_t tex);
int map_add_interactable(map_t *map, float x, float y, tex_t tex, u8 type,
	u8 *data);


// none of these functions do bounds checks (except obv map_is_in_bounds)
sprite_t *map_get_interactable(map_t *map, size_t cx, size_t cy);

bool map_is_in_bounds(map_t *map, int x, int y);
cell_t *map_get_cell(map_t *map, size_t x, size_t y);
bool is_same_cell(vec3_t c1, vec3_t c2);



void map_print_debug_info(map_t *map);
void map_render_minimap(map_t *map, char *file_name);


#endif
