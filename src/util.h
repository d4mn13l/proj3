#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>



#define ASSERT(a, cmp, b, line, file)                            \
if (!(a cmp b)) {                                                \
	printf("assertion \"%s %s %s\" failed at line %d in %s", \
		#a, #cmp, #b, line, file);                       \
	exit(EXIT_FAILURE);                                      \
}


#endif
