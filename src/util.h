#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include "shared.h"

//#define DEBUG

#ifdef __3DS__
#define SIZE_T_FORMAT "%u"
#define EOF_REAL 255
// 255 seems to be the EOF value on the 3ds, but the EOF macro isnt properly set
#else
#define SIZE_T_FORMAT "%lu"
#define EOF_REAL EOF
#endif


#define ARRAY_LENGTH(arr) sizeof(arr) / sizeof(arr[0])


#ifdef DEBUG
#define ASSERT(cond) do {                                               \
if (!(cond)) {                                                          \
	if (g_log_file)                                                 \
	fprintf(g_log_file, "assertion \"%s\" failed at line %d in %s", \
		#cond, __LINE__, __FILE__);                             \
	exit(EXIT_FAILURE);                                             \
}} while (0);
#define ASSERT_MSG(cond, msg) do {                                           \
if (!(cond)) {                                                               \
	if (g_log_file)                                                      \
	fprintf(g_log_file, "assertion \"%s\" failed at line %d in %s (%s)", \
		#cond, __LINE__, __FILE__, msg);                             \
	exit(EXIT_FAILURE);                                                  \
}} while (0);

#else
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg)
#endif


// these are for asserts that should happen even in non debug builds
#define ASSERT_ALWAYS(cond) do {                                     \
if (!(cond)) {                                                        \
	if (g_log_file)                                                \
	fprintf(g_log_file, "assertion \"%s\" failed at line %d in %s", \
		#cond, __LINE__, __FILE__);                              \
	exit(EXIT_FAILURE);                                               \
}} while (0);
#define ASSERT_ALWAYS_MSG(cond, msg) do           {                          \
if (!(cond)) {                                                               \
	if (g_log_file)                                                      \
 	fprintf(g_log_file, "assertion \"%s\" failed at line %d in %s (%s)", \
		#cond, __LINE__, __FILE__, msg);                             \
	exit(EXIT_FAILURE);                                                  \
}} while (0);




#define UNREACHABLE(msg) do {                                                \
	if (g_log_file)                                                      \
	fprintf(g_log_file, "unreachable code reached at line %d in %s: %s", \
		__LINE__, __FILE__, msg);                                    \
	exit(1);                                                             \
} while (0);

#ifdef ENABLE_RENDER_DEBUGGING
#define RENDER_DEBUG(x) x
#else
#define RENDER_DEBUG(x)
#endif

#endif
