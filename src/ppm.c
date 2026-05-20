#include "ppm.h"

#include <stdio.h>
#include <stdint.h>

void ppm_write_header(FILE *f, int w, int h) {
	fprintf(f, "P6\n%d %d\n%d\n", w, h, PPM_MAX_COLOUR);
}

void ppm_write_colour(FILE *f, uint8_t r, uint8_t g, uint8_t b) {
	fprintf(f, "%c%c%c", r, g, b);
}
