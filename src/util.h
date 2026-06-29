#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>

//#define DEBUG

#ifdef __3DS__
#define SIZE_T_FORMAT "%u"
#define EOF_REAL 255
// 255 seems to be the EOF value on the 3ds, but the EOF macro isnt properly set
#else
#define SIZE_T_FORMAT "%lu"
#define EOF_REAL EOF
#endif

#ifdef DEBUG
#define ASSERT(cond, line, file) do {                      \
if (!(cond)) {                                             \
	printf("assertion \"%s\" failed at line %d in %s", \
		#cond, line, file);                        \
	exit(EXIT_FAILURE);                                \
}} while (0);


#define ASSERT_RET(cond, line, file) do {                  \
if (!(cond)) {                                             \
	printf("assertion \"%s\" failed at line %d in %s", \
		#cond, line, file);                        \
	return EXIT_FAILURE;                               \
} while (0);
#else
#define ASSERT(cond, line, file)
#define ASSERT_RET(cond, line, file)
#endif


// this is for asserts that should happen even in prod builds
#define ASSERT_ALWAYS(cond, line, file) do {                      \
if (!(cond)) {                                                    \
	printf("assertion \"%s\" failed at line %d in %s",        \
		#cond, line, file);                               \
	exit(EXIT_FAILURE);                                       \
}} while (0);




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
