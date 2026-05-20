#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


// this doesnt free map->cells if it already exists, bc how would you check
// for that
int map_load(map_t *map, FILE *f) {
	fscanf(f, "%d %d", &map->w, &map->h);

	map->cells = malloc(map->w * map->h * sizeof(cell_t));
	ASSERT(map->cells, !=, NULL, __LINE__, __FILE__);

	fgetc(f);
	// skip over the newline after the dimensions in line 1
	
	char c;
	size_t i = 0;

	while ((c = fgetc(f)) != EOF) {
		if (c == '\n') {
			ASSERT((i % map->w), ==, 0, __LINE__, __FILE__" (line in map file too long/short)");
			// im not sure if in this case the file is invalid or not
			// according to the specification it should be but the example doesnt
			// contain spaces at the end of lines
			continue;
		}
		
		if (c == CELL_CHAR_EMPTY) {
			map->cells[i] = CELL_EMPTY;
		} else if (c == CELL_CHAR_UNTEXTURED_WALL) {
			map->cells[i] = CELL_UNTEXTURED_WALL;
		} else if (c == CELL_CHAR_PLAYER_START) {
			map->cells[i] = CELL_EMPTY;
			map->player_start_x = i % map->w;
			map->player_start_y = i / map->w;
		} else {
			int tex = c - '0';
			ASSERT(tex, >=, 0, __LINE__, __FILE__);
			ASSERT(tex, <, 10, __LINE__, __FILE__);
			map->cells[i] = tex;
		}

		i++;
	}

	return EXIT_SUCCESS;
}


void map_free(map_t *map) {
	free(map->cells);
}


void map_print_debug_info(map_t *map) {
	int wall_count = 0;
	int cell_count = map->w * map->h;
	
	for (size_t i = 0; i < cell_count; i++) {
		if (map->cells[i] != CELL_EMPTY) {
			wall_count++;
		}
	}
	
	printf("Map dimensions: %d x %d\n" \
		"Start point: (%d, %d)\n" \
		"Total wall cells: %d\n",
		map->w, map->h, map->player_start_x, map->player_start_y, wall_count);
}
