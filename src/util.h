#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include "renderer.h"



#define ASSERT(cond, line, file)                           \
if (!(cond)) {                                             \
	printf("assertion \"%s\" failed at line %d in %s", \
		#cond, line, file);                        \
	exit(EXIT_FAILURE);                                \
}



#define ASSERT_RET(cond, line, file)                       \
if (!(cond)) {                                             \
	printf("assertion \"%s\" failed at line %d in %s", \
		#cond, line, file);                        \
	return EXIT_FAILURE;                               \
}

#define UNREACHABLE(line, file, msg) do {                       \
	printf("unreachable code reached at line %d in %s: %s", \
		line, file, msg);                               \
	exit(1);                                                \
} while (0);

#ifdef ENABLE_RENDER_DEBUGGING
#define RENDER_DEBUG(x) x
#else
#define RENDER_DEBUG(x)
#endif

#endif
