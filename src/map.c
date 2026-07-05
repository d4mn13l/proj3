#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell_enter_actions.h"
#include "maths.h"
#include "ppm.h"
#include "renderer.h"
#include "shared.h"
#include "util.h"



void cell_init(cell_t *cell, tex_t tex, u8 roam_path) {
	cell->tex = tex;
	memset(&cell->sprites, 0, sizeof(cell->sprites[0]) * MAX_SPRITES_PER_CELL);
	cell->flags = CELL_FLAGS_NONE;
	cell->roam_path = 0;
	cell->cell_enter_action = NULL;
}

void map_load(map_t *map, FILE *f) {
	ASSERT_ALWAYS_MSG(g_game.ta.tex != NULL, "load ta before loading map");
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
			cell_init(&map->cells[i], CELL_TEX_EMPTY, 0);
		} else if ('a' <= c && c <= 'z') {
			// creature path and empty
			cell_init(&map->cells[i], CELL_TEX_EMPTY, c - 'a' + 1);
		} else if (c == CELL_CHAR_PLAYER_START) {
			cell_init(&map->cells[i], CELL_TEX_EMPTY, 0);
			map->player_start_x = i % map->w;
			map->player_start_y = i / map->w;
		} else if ('A' <= c && c <= 'Z') {
			// creature path and vent
			cell_init(&map->cells[i], CELL_TEX_VENT, c - 'A' + 1);
		} else if ('0' <= c && c <= '9' ){
			cell_init(&map->cells[i], c - '0', 0);
		} else {
			cell_init(&map->cells[i], CELL_TEX_DEFAULT, 0);
		}

		i++;
	}
	ASSERT_MSG(i == map->w * map->h, "expected more map, found EOF/]");

	// cell enter action section
	while (true) {
		size_t cx, cy;
		char name[32];
		int res = fscanf(f, SIZE_T_FORMAT " " SIZE_T_FORMAT " %31s",
			&cx, &cy, name);
		if (res == 0 || res == -1) break;
		ASSERT_ALWAYS_MSG(res == 3, "failed to parse cell enter action in map file");

		cell_t *cell = map_get_cell(map, cx, cy);
		
		for (int i = 0; i < ARRAY_LENGTH(cell_enter_actions); i += 2) {
			if (strcmp(name, cell_enter_actions[i].name)) continue;

			cell->cell_enter_action =
				cell_enter_actions[i + 1].action;
			break;
		}

		ASSERT_ALWAYS_MSG(cell->cell_enter_action != NULL, "didnt find cell enter action for name");
	
		char c;
		do { c = fgetc(f); }
		while (c != '\n' && c != EOF_REAL);
	}
	
	while (fgetc(f) != MAP_SECTION_SEPERATOR);


	// sprite section
	while(true) {
		float x, y;
		u8 tex_x, tex_y;
		int res = fscanf(f, "%f %f %hhu %hhu", &x, &y, &tex_x, &tex_y);
		if (res == 0 || res == -1) break;
		ASSERT_ALWAYS_MSG(res == 4, "failed to parse sprite in map file");
		map_add_sprite(map, x, y, tex_x + tex_y * g_game.ta.nx);

		char c;
		do { c = fgetc(f); }
		while (c != '\n' && c != EOF_REAL);
	}

	while (fgetc(f) != MAP_SECTION_SEPERATOR);
	// TODO test this
	// this should read the newline and the section separator?

	// interactables section
	 
	while (true) {
		float x, y;
		u8 tex_x, tex_y, interactable_type;
		u8 data[SPRITE_DATA_COUNT];

		int res = fscanf(f, "%hhu %f %f %hhu %hhu",
			&interactable_type, &x, &y, &tex_x, &tex_y);
		if (res == 0 || res == -1) break;
		ASSERT_ALWAYS_MSG(res == 5, "failed to parse interactable in map file");
		for (int i = 0; i < SPRITE_DATA_COUNT; i++) {
			res = fscanf(f, "%hhu", &data[i]);
			ASSERT_ALWAYS_MSG(res == 1, "missing data in interactable definition in map file");
		}
		map_add_interactable(map, x, y, tex_x + tex_y * g_game.ta.nx,
			interactable_type, data);

		char c;
		do { c = fgetc(f); }
		while (c != '\n' && c != EOF_REAL);
	}
}


void map_free(map_t *map) {
	free(map->cells);
	map->cells = NULL;
}


int map_add_sprite(map_t *map, float x, float y, tex_t tex) {
	size_t cx = (int) x;
	size_t cy = (int) y;
	
	cell_t *cell = map_get_cell(map, cx, cy);

	for (size_t i = 1; i < MAX_SPRITES_PER_CELL; i++) {
		// start at 1 because 0 is reserved for the creature
		sprite_t *sprite = &cell->sprites[i];
		if (sprite->tex != 0) continue;
			// using tex to see if there is a sprite  at that
			// location should be fine because tex 0 is the ceiling
			// tex so it will never be used in a sprite
		sprite->pos = (vec3_t) {x, y, CAMERA_HEIGHT};
		sprite->width = 0.5; // TODO implement this
		sprite->tex = tex;
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

int map_add_interactable(map_t *map, float x, float y, tex_t tex, u8 type,
		u8 *data) {
	// duplicated from map_add_sprite (good practice)
	size_t cx = (int) x;
	size_t cy = (int) y;
	
	cell_t *cell = map_get_cell(map, cx, cy);

	for (size_t i = 1; i < MAX_SPRITES_PER_CELL; i++) {
		// start at 1 because 0 is reserved for the creature
		sprite_t *sprite = &cell->sprites[i];
		if (sprite->tex != 0) continue;
			// using tex to see if there is a sprite at that
			// location should be fine because tex 0 is the ceiling
			// tex so it will never be used in a sprite
		sprite->pos = (vec3_t) {x, y, CAMERA_HEIGHT};
		sprite->width = 0.5; // TODO implement this
		sprite->tex = tex;
		sprite->interactable_type = type;
		for (int ci = 0; ci < SPRITE_DATA_COUNT; ci++) {
			sprite->data[ci] = data[ci];
		}
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}


void map_entered_cell(map_t *map, vec3_t at) {
	cell_t *cell = map_get_cell(map, at.x, at.y);
	if (cell->cell_enter_action) {
		cell->cell_enter_action(cell,
			(vec3_t) {(size_t) at.x + 0.5, (size_t) at.y + 0.5, 0.5});
	}
}


bool map_is_in_bounds(map_t *map, int x, int y) {
	return 0 <= x && x < map->w && 0 <= y && y < map->h;
}


cell_t *map_get_cell(map_t *map, size_t x, size_t y) {
	return &map->cells[x + y * map->w];
}


bool is_same_cell(vec3_t c1, vec3_t c2) {
	return (int) c1.x == (int) c2.x && (int) c1.y == (int) c2.y;
}


sprite_t *map_get_interactable(map_t *map, size_t cx, size_t cy) {
	cell_t *cell = map_get_cell(map, cx, cy);
	for (size_t i = 0; i < MAX_SPRITES_PER_CELL; i++) {
		if (cell->sprites[i].interactable_type != INTERACTABLE_TYPE_NONE)
			return &cell->sprites[i];
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
