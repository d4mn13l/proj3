#include "ppm.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"


void ppm_image_init(image_t *img, size_t w, size_t h) {
	img->w = w;
	img->h = h;
	img->pixels = malloc(w * h * sizeof(colour_t));
	ASSERT(img->pixels != NULL, __LINE__, __FILE__);
}

image_t *ppm_image_new(size_t w, size_t h) {
	image_t *img = malloc(sizeof(image_t));
	ASSERT(img != NULL, __LINE__, __FILE__);
	ppm_image_init(img, w, h);
	return img;
}


void ppm_write_header(FILE *f, size_t w, size_t h) {
	fprintf(f, "P6\n%lu %lu\n%d\n", w, h, PPM_MAX_COLOUR);
}

void ppm_write_colour(FILE *f, colour_t c) {
	fprintf(f, "%c%c%c", c.r, c.g, c.b);
}

image_t *ppm_image_load(FILE *f) {
	size_t w, h;
	char format[3] = {0};
	int max_colour;

	// put the ppm header in this buffer
	// this allows to ignore comments
	char header_buf[128] = {'\0'};
	int read_elements_count = 0;
	char c;
	for (size_t i = 0; i < 127; i++) {
		if (read_elements_count == 4) {
			break;
		}
		c = fgetc(f);
		ASSERT_ALWAYS(!feof(f), __LINE__, __FILE__" (found eof while parsing ppm header)");
		if (isspace(c)) {
			// skip ahead to next non-whitespace character
			read_elements_count++;
			while(isspace(fgetc(f)));
			header_buf[i] = ' ';
			// we also read the first non-whitespace character, so we need to move
			// the file pointers current position back by 1
			fseek(f, -1, SEEK_CUR);
		} else if (c == '#') {
			// skip lines starting with #
			while((fgetc(f)) != '\n');
			header_buf[i] = ' ';
		} else {
			header_buf[i] = c;
		}
	}

	int sscanf_res = sscanf(header_buf, "%2s %lu %lu %d", format, &w, &h, &max_colour);
	ASSERT_ALWAYS(sscanf_res == 4, __LINE__, __FILE__" (failed to parse ppm header)");
	ASSERT_ALWAYS(!strcmp(format, PPM_FORMAT), __LINE__, __FILE__);
	ASSERT_ALWAYS(max_colour <= PPM_MAX_COLOUR, __LINE__, __FILE__);
	

	image_t *img = ppm_image_new(w, h);

	// since the pixels in the file are in binary form we can load it like this
	fread(img->pixels, sizeof(colour_t), w * h, f);

	ASSERT_ALWAYS(fgetc(f) == EOF, __LINE__, __FILE__" (in image file, expected eof)");
	return img;
}

void ppm_image_write(FILE *f, image_t *img) {
	ppm_write_header(f, img->w, img->h);
	// dump the pixels to the file
	fwrite(img->pixels, sizeof(colour_t), img->h * img->w, f);
}
