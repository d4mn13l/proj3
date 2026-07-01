#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maths.h"
#include "ppm.h"
#include "renderer.h"
#include "util.h"



void cell_init(cell_t *cell, tex_t tex) {
	cell->tex = tex;
	memset(&cell->props, 0, sizeof(cell->props[0]) * MAX_PROPS_PER_CELL);
	cell->flags = CELL_FLAGS_NONE;
}

void map_load(map_t *map, FILE *f) {
	fscanf(f, SIZE_T_FORMAT" "SIZE_T_FORMAT, &map->w, &map->h);

	map->cells = malloc(map->w * map->h * sizeof(cell_t));
	ASSERT_ALWAYS(map->cells != NULL);

	fgetc(f);
	// skip over the newline after the dimensions in line 1
	
	unsigned char c;
	size_t i = 0;

	// parse map section
	while ((c = fgetc(f)) != MAP_SECTION_SEPERATOR) {
		if (c == EOF_REAL) break;
		ASSERT_ALWAYS_MSG(i <= map->w * map->h, "expected end of map file, found more map");
		ASSERT_ALWAYS_MSG(!feof(f), "expected more map, found eof");

		if (c == '\n') {
			ASSERT_ALWAYS_MSG((i % map->w) == 0, "line in map file too long/short");
			continue;
		}

		if (c == CELL_CHAR_EMPTY) {
			cell_init(&map->cells[i], CELL_TEX_EMPTY);
		} else if (c == CELL_CHAR_PLAYER_START) {
			cell_init(&map->cells[i], CELL_TEX_EMPTY);
			map->player_start_x = i % map->w;
			map->player_start_y = i / map->w;
		} else {
			int tex = c - '0';
			if (0 <= tex && tex < 10) {
				cell_init(&map->cells[i], tex);
			} else {
				cell_init(&map->cells[i], CELL_TEX_UNSPECIFIED);
			}
		}

		i++;
	}
	ASSERT_MSG(i == map->w * map->h, "expected more map, found EOF/]");

	// props section

	for (c = fgetc(f); c != MAP_SECTION_SEPERATOR && c != EOF_REAL; c = fgetc(f)) {
		// puts("notlop");
		float x, y;
		u8 tex;
		int res = fscanf(f, "%f %f %hhu", &x, &y, &tex);
		if (res == 0 || res == -1) break;
		ASSERT_ALWAYS_MSG(res == 3, "failed to parse prop in map file");
		map_add_prop(map, x, y, tex);
	}
	fgetc(f);

	// interactables section
	 
	puts("int");
	for (c = fgetc(f); c != MAP_SECTION_SEPERATOR && c != EOF_REAL; c = fgetc(f)) {
		puts("lop");
		float x, y;
		u8 tex, interactable_type;
		u8 data[PROP_DATA_COUNT];

		int res = fscanf(f, "%f %f %hhu %hhu",
			&x, &y, &tex, &interactable_type);
		if (res == 0 || res == -1) break;
		ASSERT_ALWAYS_MSG(res == 4, "failed to parse interactable in map file");
		for (int i = 0; i < PROP_DATA_COUNT; i++) {
			res = fscanf(f, "%hhu", &data[i]);
			printf("scanned data[%d] = %d\n", i, data[i]);
			ASSERT_ALWAYS_MSG(res == 1, "missing data in interactable definition in map file");
		}
		map_add_interactable(map, x, y, tex, interactable_type, data);
	}
}


void map_free(map_t *map) {
	free(map->cells);
	map->cells = NULL;
}


int map_add_prop(map_t *map, float x, float y, tex_t tex) {
	size_t cx = (int) x;
	size_t cy = (int) y;
	
	cell_t *cell = map_get_cell(map, cx, cy);

	for (size_t i = 0; i < MAX_PROPS_PER_CELL; i++) {
		prop_t *prop = &cell->props[i];
		if (prop->tex != 0) continue;
			// using tex to see if there is a prop at that location
			// should be fine because tex 0 is the ceiling tex
		prop->pos = (vec3_t) {x, y, CAMERA_HEIGHT};
		prop->width = 0.5; // TODO implement this
		prop->tex = tex;
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

int map_add_interactable(map_t *map, float x, float y, tex_t tex, u8 type,
		u8 *data) {
	// duplicated from map_add_prop (good practice)
	size_t cx = (int) x;
	size_t cy = (int) y;
	
	cell_t *cell = map_get_cell(map, cx, cy);

	for (size_t i = 0; i < MAX_PROPS_PER_CELL; i++) {
		prop_t *prop = &cell->props[i];
		if (prop->tex != 0) continue;
			// using tex to see if there is a prop at that location
			// should be fine because tex 0 is the ceiling tex
		prop->pos = (vec3_t) {x, y, CAMERA_HEIGHT};
		prop->width = 0.5; // TODO implement this
		prop->tex = tex;
		prop->interactable_type = type;
		for (int ci = 0; ci < PROP_DATA_COUNT; ci++) {
			prop->data[ci] = data[ci];
		}
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
	
}


bool map_is_in_bounds(map_t *map, int x, int y) {
	return 0 <= x && x < map->w && 0 <= y && y < map->h;
}


cell_t *map_get_cell(map_t *map, size_t x, size_t y) {
	return &map->cells[x + y * map->w];
}

prop_t *map_get_interactable(map_t *map, size_t cx, size_t cy) {
	cell_t *cell = map_get_cell(map, cx, cy);
	for (size_t i = 0; i < MAX_PROPS_PER_CELL; i++) {
		if (cell->props[i].interactable_type != INTERACTABLE_TYPE_NONE)
			return &cell->props[i];
	}
	return NULL;
}



void map_print_debug_info(map_t *map) {
	unsigned int wall_count = 0;
	size_t cell_count = map->w * map->h;
	
	for (size_t i = 0; i < cell_count; i++) {
		if (map->cells[i].tex != CELL_TEX_EMPTY) {
			wall_count++;
		}
	}
	
	printf("Map dimensions: "SIZE_T_FORMAT" x "SIZE_T_FORMAT"\n" \
		"Start point: ("SIZE_T_FORMAT", "SIZE_T_FORMAT")\n" \
		"Total wall cells: %u\n",
		map->w, map->h, map->player_start_x, map->player_start_y, wall_count);
}


void map_render_minimap(map_t *map, char *file_name) {
	FILE *f = fopen(file_name, "w");
	ASSERT(f != NULL);

	ppm_write_header(f, map->w, map->h);

	size_t cell_count = map->w * map->h;
	size_t player_start_i = map->player_start_y * map->w + map->player_start_x;

	for (size_t i = 0; i < cell_count; i++) {
		if (i == player_start_i) {
			ppm_write_colour(f, COLOUR_GREEN);
		} else if (map->cells[i].tex == CELL_TEX_EMPTY) {
			ppm_write_colour(f, COLOUR_WHITE);
		} else {
			ppm_write_colour(f, COLOUR_BLACK);
		}
	}

	fclose(f);
}
