#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define CELL_CHAR_EMPTY ' '
#define CELL_CHAR_PLAYER_START 'S'

typedef int8_t cell_t;

enum {
	CELL_EMPTY = -1,
	// indeces from 0 to 9 are used for textured walls
	CELL_UNTEXTURED_WALL = 10,
};


typedef struct {
	unsigned int w, h;
	unsigned int player_start_x, player_start_y;
	cell_t *cells;
} map_t;



void map_load(map_t *map, FILE *map_file);
void map_free(map_t *map);

void map_print_debug_info(map_t *map);
void map_render_minimap(map_t *map, char *file_name);


bool map_is_in_bounds(map_t *map, int x, int y);
cell_t map_get_cell(map_t *map, int x, int y);
// this does not perform any bound checks

#endif
