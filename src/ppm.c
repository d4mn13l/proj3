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

void ppm_image_free(image_t *img) {
	free(img->pixels);
}


image_t** ppm_image_split(image_t *src, size_t nx, size_t ny) {
	size_t into_w = src->w / nx;
	size_t into_h = src->h / ny;
	ASSERT(into_w * nx == src->w, __LINE__, __FILE__);
	ASSERT(into_h * ny == src->h, __LINE__, __FILE__);
	// make sure that src->w is a multiple of w and src->h is multiple of h
	// so that the image can actually be split up equally

	size_t into_count = nx * ny;

	image_t **into = malloc(into_count * sizeof(image_t));

	// this just loops over into, initialises the target images and loops
	// over the pixels to copy them over from src
	// naming:
	// ix/iy: x/y position of the current subimages
	// px/py: x/y position of the current pixel
	for (size_t iy = 0; iy < ny; iy++) {
		for (size_t ix = 0; ix < nx; ix++) {
			size_t cur_img_i = ix + iy * nx;
			into[cur_img_i] = ppm_image_new(into_w, into_h);

			size_t into_pix_i = 0;
			for (size_t py = iy * into_h; py < (iy+1) * into_h; py++) {
				for (size_t px = ix * into_w; px < (ix+1) * into_w; px++) {
					into[cur_img_i]->pixels[into_pix_i] =
						src->pixels[px + py * src->w];
					into_pix_i++;
				}
			}
		}
	}

	return into;
}


void ppm_write_header(FILE *f, size_t w, size_t h) {
	fprintf(f, "P6\n%lu %lu\n%d\n", w, h, PPM_MAX_COLOUR);
}

void ppm_write_colour(FILE *f, colour_t c) {
	fprintf(f, "%c%c%c", c.r, c.g, c.b);
}

void ppm_image_load(image_t *img, FILE *f) {
	size_t w, h;
	char format[3] = {0};
	int max_colour;

	// parsing the header is a bit tricky since the standart allows for any
	// whitespace to be between values, but comments skip entire lines
	// the best solution i could think of is to copy the header into a
	// buffer while ignoring comments and then use sscanf to get the values
	// from the header
	char header_buf[128] = {'\0'};
	int read_elements_count = 0;
	char c;
	for (size_t i = 0; i < 128; i++) {
		if (read_elements_count == 4) {
			goto header_copy_success;
		}
		c = fgetc(f);
		ASSERT_ALWAYS(!feof(f), __LINE__, __FILE__ \
			        " (found eof while parsing ppm header)");
		if (isspace(c)) {
			// skip ahead to next non-whitespace character
			read_elements_count++;
			while(isspace(fgetc(f)));
			header_buf[i] = ' ';
			// we also read the first non-whitespace character, so
			// we need to move the file pointers current position
			// back by 1
			fseek(f, -1, SEEK_CUR);
		} else if (c == '#') {
			// skip lines starting with #
			while((fgetc(f)) != '\n');
			header_buf[i] = ' ';
		} else {
			header_buf[i] = c;
		}
	}
	// this will be skipped if the 4 header elements are found before the
	// buffer is filled
	UNREACHABLE(__LINE__, __FILE__, "failed to parse ppm header (too long)");

	header_copy_success:
	int r = sscanf(header_buf, "%2s %lu %lu %d", format, &w, &h, &max_colour);
	ASSERT_ALWAYS(r == 4, __LINE__, __FILE__" (failed to parse ppm header)");
	ASSERT_ALWAYS(!strcmp(format, PPM_FORMAT), __LINE__, __FILE__);
	ASSERT_ALWAYS(max_colour <= PPM_MAX_COLOUR, __LINE__, __FILE__);

	// initialise the image
	ppm_image_init(img, w, h);

	// the pixels in the file are in binary form so we can load it like this
	fread(img->pixels, sizeof(colour_t), w * h, f);

	ASSERT_ALWAYS(fgetc(f) == EOF, __LINE__, __FILE__ \
			" (when loading an image file, expected eof)");
}

void ppm_image_write(image_t *img, FILE *f) {
	ppm_write_header(f, img->w, img->h);
	// dump the pixels to the file
	fwrite(img->pixels, sizeof(colour_t), img->h * img->w, f);
}
