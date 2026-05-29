#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>

//#define DEBUG


#ifdef DEBUG
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
#else
#define ASSERT(cond, line, file)
#define ASSERT_RET(cond, line, file)
#endif


// this is for asserts that should happen even in prod builds
#define ASSERT_ALWAYS(cond, line, file)                           \
if (!(cond)) {                                                    \
	printf("assertion \"%s\" failed at line %d in %s",        \
		#cond, line, file);                               \
	exit(EXIT_FAILURE);                                       \
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
