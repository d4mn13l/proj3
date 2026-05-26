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
void ppm_image_free(image_t *img);

image_t **ppm_image_split(image_t *src, size_t nx, size_t ny);
// splits the image into nx * ny images  of equal size and returns them as an
// array, for example:
// nx = 4, ny = 2, then the image would be split like this
// ---------
// |0|1|2|3|
// ---------
// |4|5|6|7|
// ---------
// where the numbers are the indeces of into

void ppm_write_header(FILE *f, size_t w, size_t h);
void ppm_write_colour(FILE *f, colour_t c);

void ppm_image_load(image_t *img, FILE *f);
void ppm_image_write(image_t *img, FILE *f);

#endif
