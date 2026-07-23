#ifndef PPM_H
#define PPM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PPM_MAX_COLOUR 255
#define PPM_FORMAT "P6"

typedef struct {
	uint8_t b, g, r;
	// the 3ds' represents colours as BGR8
} colour_t;

typedef struct {
	size_t w, h;
	colour_t *pixels;
	// dont use this directly bc the order is different from what the rest
	// of the code uses, use ppm_image_[get,set]_pixel instead
} image_t;

#define COLOUR_WHITE (colour_t) {PPM_MAX_COLOUR, PPM_MAX_COLOUR, PPM_MAX_COLOUR}
#define COLOUR_BLU (colour_t) {PPM_MAX_COLOUR, 0, 0}
#define COLOUR_GREEN (colour_t) {0, PPM_MAX_COLOUR, 0}
#define COLOUR_RED (colour_t) {0, 0, PPM_MAX_COLOUR}
#define COLOUR_BLACK (colour_t) {0, 0, 0}
#define COLOUR_TRANSPARENT (colour_t) {0x88, 0, 0x98}


image_t *ppm_image_new(size_t w, size_t h);
void ppm_image_init(image_t *img, size_t w, size_t h);
void ppm_image_free(image_t *img);

bool ppm_colour_equals(colour_t c1, colour_t c2);

void ppm_image_set_pixel(image_t *img, size_t x, size_t y, colour_t to);
void ppm_image_fill_rectangle(image_t *img, size_t px, size_t py, size_t sx,
        size_t sy, colour_t colour);
colour_t* ppm_image_get_pixel(image_t *img, size_t x, size_t y);


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

bool ppm_image_is_in_bounds(image_t *img, size_t x, size_t y);

void ppm_write_header(FILE *f, size_t w, size_t h);
void ppm_write_colour(FILE *f, colour_t c);

void ppm_image_load(image_t *img, FILE *f);
void ppm_image_write(image_t *img, FILE *f);
void ppm_image_write_pixels(image_t *img, FILE *f);


#endif
