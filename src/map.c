#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppm.h"
#include "util.h"


// this doesnt free map->cells if it already exists, bc how would you check
// for that
void map_load(map_t *map, FILE *f) {
	fscanf(f, "%lu %lu", &map->w, &map->h);

	map->cells = malloc(map->w * map->h * sizeof(cell_t));
	ASSERT(map->cells != NULL, __LINE__, __FILE__);

	fgetc(f);
	// skip over the newline after the dimensions in line 1
	
	char c;
	size_t i = 0;

	while ((c = fgetc(f)) != EOF) {
		ASSERT(i <= map->w * map->h, __LINE__,
			__FILE__" (expected end of map file, found more map)");

		if (c == '\n') {
			ASSERT((i % map->w) == 0, __LINE__,
				__FILE__" (line in map file too long/short)");
			// im not sure if in this case the file is invalid or not
			// according to the specification it should be but the example doesnt
			// contain spaces at the end of lines
			continue;
		}
		
		if (c == CELL_CHAR_EMPTY) {
			map->cells[i] = CELL_EMPTY;
		} else if (c == CELL_CHAR_PLAYER_START) {
			map->cells[i] = CELL_EMPTY;
			map->player_start_x = i % map->w;
			map->player_start_y = i / map->w;
		} else {
			int tex = c - '0';
			if (0 <= tex && tex < 10) {
				map->cells[i] = tex;
			} else {
				map->cells[i] = CELL_UNSPECIFIED_TEX;
			}
		}

		i++;
	}

	ASSERT(i == map->w * map->h, __LINE__,
		__FILE__" (expected more lines in map file, found EOF)")
	// i cant be greater than w*h bc that is already checked in the loop
}


void map_free(map_t *map) {
	free(map->cells);
	map->cells = NULL;
}


void map_print_debug_info(map_t *map) {
	unsigned int wall_count = 0;
	size_t cell_count = map->w * map->h;
	
	for (size_t i = 0; i < cell_count; i++) {
		if (map->cells[i] != CELL_EMPTY) {
			wall_count++;
		}
	}
	
	printf("Map dimensions: %lu x %lu\n" \
		"Start point: (%lu, %lu)\n" \
		"Total wall cells: %u\n",
		map->w, map->h, map->player_start_x, map->player_start_y, wall_count);
}


void map_render_minimap(map_t *map, char *file_name) {
	FILE *f = fopen(file_name, "w");
	ASSERT(f != NULL, __LINE__, __FILE__);

	ppm_write_header(f, map->w, map->h);

	size_t cell_count = map->w * map->h;
	size_t player_start_i = map->player_start_y * map->w + map->player_start_x;
	
	for (size_t i = 0; i < cell_count; i++) {
		if (i == player_start_i) {
			ppm_write_colour(f, COLOUR_GREEN);
		} else if (map->cells[i] == CELL_EMPTY) {
			ppm_write_colour(f, COLOUR_WHITE);
		} else {
			ppm_write_colour(f, COLOUR_BLACK);
		}
	}

	fclose(f);
}


bool map_is_in_bounds(map_t *map, size_t x, size_t y) {
	return 0 <= x && x < map->w && 0 <= y && y < map->h;
}


cell_t map_get_cell(map_t *map, size_t x, size_t y) {
	return map->cells[x + y * map->w];
}
