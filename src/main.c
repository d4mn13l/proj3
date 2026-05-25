#include <stdio.h>
#include <string.h>

#include "map.h"
#include "renderer.h"
#include "util.h"


int main(int argc, char *argv[]) {
	ASSERT(argc >= 2, __LINE__, __FILE__);

	FILE *map_file = fopen(argv[1], "r");
	ASSERT(map_file != NULL, __LINE__, __FILE__);

	map_t map;
	map_load(&map, map_file);

	fclose(map_file);
	map_file = NULL;
	
	if (!strcmp(argv[2], "-I")) {
		map_print_debug_info(&map);
	} else if (!strcmp(argv[2], "-M")) {
		ASSERT(argc == 4, __LINE__, __FILE__);
		map_render_minimap(&map, argv[3]);
	} else if (!strcmp(argv[2], "-R")) {
		ASSERT(argc == 10, __LINE__, __FILE__);
		
		int width = atoi(argv[4]);
		int height = atoi(argv[5]);
		double fov = atof(argv[6]);
		double rotation = atof(argv[7]);
		double px = atof(argv[8]);
		double py = atof(argv[9]);
	
		FILE *f = fopen(argv[3], "w");
		ASSERT(f != NULL, __LINE__, __FILE__);
		render(f, NULL, &map, width, height, px, py, fov, rotation);
	} else {
		printf("unknown option %s", argv[2]);
	}

	map_free(&map);

	return 0;
}
