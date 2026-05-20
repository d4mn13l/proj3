#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stdio.h>

#define PPM_MAX_COLOUR 255


void ppm_write_header(FILE *f, int w, int h);
void ppm_write_colour(FILE *f, uint8_t r, uint8_t g, uint8_t b);


#endif
