#include <stdio.h>
#include <string.h>

#include "map.h"
#include "maths.h"
#include "ppm.h"
#include "renderer.h"
#include "util.h"


int main(int argc, char *argv[]) {
	ASSERT_ALWAYS(argc >= 2, __LINE__, __FILE__);

	FILE *map_file = fopen(argv[1], "r");
	ASSERT_ALWAYS(map_file != NULL, __LINE__, __FILE__);

	map_t map;
	map_load(&map, map_file);

	fclose(map_file);
	map_file = NULL;
	
	if (!strcmp(argv[2], "-I")) {
		map_print_debug_info(&map);
	} else if (!strcmp(argv[2], "-M")) {
		ASSERT_ALWAYS(argc == 4, __LINE__, __FILE__);
		map_render_minimap(&map, argv[3]);
	} else if (!strcmp(argv[2], "-R")) {
		ASSERT_ALWAYS(argc == 10, __LINE__, __FILE__);
		
		size_t width, height;
		sscanf(argv[4], "%lu", &width);
		sscanf(argv[5], "%lu", &height);
		double fov = atof(argv[6]);
		double rotation = atof(argv[7]);
		double px = atof(argv[8]);
		double py = atof(argv[9]);
	
		image_t img;
		ppm_image_init(&img, width, height);
		render(&img, &map, px, py, deg_to_rad(fov),
			deg_to_rad(rotation), draw_untextured, NULL,
			DRAW_FLAGS_EMPTY);
		FILE *f = fopen(argv[3], "w");
		ASSERT_ALWAYS(f != NULL, __LINE__, __FILE__);
		ppm_image_write(&img, f);
		
		fclose(f);
		ppm_image_free(&img);
	} else if (!strcmp(argv[2], "-T")) {
		// TODO handle invalid arguments
		size_t tex_count_x, tex_count_y, w, h;
		sscanf(argv[4], "%lu", &tex_count_x);
		sscanf(argv[5], "%lu", &tex_count_y);
		sscanf(argv[7], "%lu", &w);
		sscanf(argv[8], "%lu", &h);
		double fov = atof(argv[9]);
		double rotation = atof(argv[10]);
		double px = atof(argv[11]);
		double py = atof(argv[12]);

		int draw_flags = DRAW_FLAGS_EMPTY;
		if (argc > 13 && atoi(argv[13]) == 1)
			draw_flags |= DRAW_FLAG_DO_SHADING;

		if (argc > 14 && atoi(argv[14]) == 1)
			draw_flags |= DRAW_FLAG_RENDER_FLOOR_CEIL;

		FILE *ta_file = fopen(argv[3], "r");
		ASSERT_ALWAYS(ta_file != NULL, __LINE__, __FILE__);
		tex_atlas_t ta;
		tex_atlas_load(&ta, ta_file, tex_count_x, tex_count_y);
		fclose(ta_file);

		image_t img;
		ppm_image_init(&img, w, h);
		render(&img, &map, px, py, deg_to_rad(fov),
			deg_to_rad(rotation), draw_textured, &ta, draw_flags);

		FILE *out = fopen(argv[6], "w");
		ASSERT_ALWAYS(out != NULL, __LINE__, __FILE__);
		ppm_image_write(&img, out);

		fclose(out);
		ppm_image_free(&img);
		tex_atlas_free(&ta);
	}
	else if (!strcmp(argv[2], "-C")) {
		ASSERT_ALWAYS(argc == 9, __LINE__, __FILE__);

		size_t tex_count_x, tex_count_y, w, h;
		sscanf(argv[4], "%lu", &tex_count_x);
		sscanf(argv[5], "%lu", &tex_count_y);
		sscanf(argv[6], "%lu", &w);
		sscanf(argv[7], "%lu", &h);
		double fov = atof(argv[8]);

		FILE *ta_file = fopen(argv[3], "r");
		ASSERT_ALWAYS(ta_file != NULL, __LINE__, __FILE__);
		tex_atlas_t ta;
		tex_atlas_load(&ta, ta_file, tex_count_x, tex_count_y);
		fclose(ta_file);

		int draw_flags = DRAW_FLAG_DO_SHADING | DRAW_FLAG_RENDER_FLOOR_CEIL;

		image_t img;
		ppm_image_init(&img, w, h);
		double px, py, rotation;
		while (true) {
			scanf("%lf %lf %lf", &px, &py, &rotation);
			render(&img, &map, px, py, deg_to_rad(fov),
				deg_to_rad(rotation), draw_textured, &ta,
				draw_flags);
			ppm_image_write_pixels(&img, stdout);
			fflush(stdout);
		}

		ppm_image_free(&img);
		tex_atlas_free(&ta);
	}
	else {
		printf("unknown option %s", argv[2]);
	}

	map_free(&map);

	return 0;
}
