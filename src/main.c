#include <stdio.h>
#include <string.h>

#include "map.h"
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
		
		int width = atoi(argv[4]);
		int height = atoi(argv[5]);
		double fov = atof(argv[6]);
		double rotation = atof(argv[7]);
		double px = atof(argv[8]);
		double py = atof(argv[9]);
	
		FILE *f = fopen(argv[3], "w");
		ASSERT_ALWAYS(f != NULL, __LINE__, __FILE__);
		image_t img;
		ppm_image_init(&img, width, height);
		render(f, &map, width, height, px, py, fov, rotation,
			draw_untextured, NULL, &img);
		ppm_image_write(&img, f);
		ppm_image_free(&img);
	}
	
	#ifdef DEBUG
	else if (!strcmp(argv[2], "-P")) {
		image_t img;
		ppm_image_load(&img, fopen(argv[3], "r"));
		FILE *out = fopen(argv[4], "w");
		ASSERT(out != NULL, __LINE__, __FILE__);
		ppm_image_write(&img, out);

		ppm_image_free(&img);
		fclose(out);
	}
	else if (!strcmp(argv[2], "-S")) {
		image_t img;
		ppm_image_load(&img, fopen(argv[3], "r"));
		size_t nx = atoi(argv[5]);
		size_t ny = atoi(argv[6]);
		image_t **split = ppm_image_split(&img, nx, ny);

		int out_file_name_length = strlen(argv[4]);
		for (size_t i = 0; i < nx * ny; i++) {
			// overwrite the last character before the extension
			// with the index
			// this is only for debugging so now one cares
			argv[4][out_file_name_length - 5] = i + 48;
			FILE *f = fopen(argv[4], "w");
			ppm_image_write(split[i], f);
			ppm_image_free(split[i]);
			free(split[i]);
		}
		ppm_image_free(&img);
	}
	#endif
	
	else {
		printf("unknown option %s", argv[2]);
	}

	map_free(&map);

	return 0;
}
