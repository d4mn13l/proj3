#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stdio.h>

#define PPM_MAX_COLOUR 255
#define PPM_FORMAT "P6"

typedef struct {
	uint8_t r,g,b;
} colour_t;

typedef struct {
	size_t w, h;
	colour_t *pixels;
} image_t;

static const colour_t COLOUR_WHITE = {PPM_MAX_COLOUR, PPM_MAX_COLOUR, PPM_MAX_COLOUR};
static const colour_t COLOUR_RED = {PPM_MAX_COLOUR, 0, 0};
static const colour_t COLOUR_GREEN = {0, PPM_MAX_COLOUR, 0};
static const colour_t COLOUR_BLUE = {0, 0, PPM_MAX_COLOUR};
static const colour_t COLOUR_BLACK = {0, 0, 0};


image_t *ppm_image_new(size_t w, size_t h);
void ppm_image_init(image_t *img, size_t w, size_t h);

void ppm_write_header(FILE *f, size_t w, size_t h);
void ppm_write_colour(FILE *f, colour_t c);

image_t *ppm_image_load(FILE *f);
void ppm_image_write(FILE *f, image_t *img);

#endif
