#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stdio.h>

#define PPM_MAX_COLOUR 255

typedef struct {
	uint8_t r,g,b;
} colour_t;


static const colour_t COLOUR_WHITE = {PPM_MAX_COLOUR, PPM_MAX_COLOUR, PPM_MAX_COLOUR};
static const colour_t COLOUR_RED = {PPM_MAX_COLOUR, 0, 0};
static const colour_t COLOUR_GREEN = {0, PPM_MAX_COLOUR, 0};
static const colour_t COLOUR_BLUE = {0, 0, PPM_MAX_COLOUR};
static const colour_t COLOUR_BLACK = {0, 0, 0};


void ppm_write_header(FILE *f, int w, int h);
void ppm_write_colour(FILE *f, colour_t c);


#endif
