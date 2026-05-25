#include "ppm.h"

#include <stdio.h>
#include <stdint.h>

void ppm_write_header(FILE *f, int w, int h) {
	fprintf(f, "P6\n%d %d\n%d\n", w, h, PPM_MAX_COLOUR);
}

void ppm_write_colour(FILE *f, colour_t c) {
	fprintf(f, "%c%c%c", c.r, c.g, c.b);
}
