#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>



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


#endif
