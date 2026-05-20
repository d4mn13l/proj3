#include <stdio.h>
#include <string.h>

#include "map.h"
#include "util.h"


int main(int argc, char *argv[]) {
	ASSERT(argc, >=, 2, __LINE__, __FILE__);

	printf("loading map from %s\n", argv[1]);

	FILE *map_file = fopen(argv[1], "r");
	ASSERT(map_file, !=, NULL, __LINE__, __FILE__);

	map_t map;
	int map_load_ret = map_load(&map, map_file);
	ASSERT(map_load_ret, ==, EXIT_SUCCESS, __LINE__, __FILE__);

	if (!strcmp(argv[2], "-I")) {
		puts("-I");
		map_print_debug_info(&map);
	} else {
		printf("unknown option %s", argv[2]);
	}

	map_free(&map);

	return 0;
}

