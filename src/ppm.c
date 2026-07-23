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
	ASSERT(img->pixels != NULL);
}

image_t *ppm_image_new(size_t w, size_t h) {
	image_t *img = malloc(sizeof(image_t));
	ASSERT(img != NULL);
	ppm_image_init(img, w, h);
	return img;
}

void ppm_image_free(image_t *img) {
	free(img->pixels);
	img->pixels = NULL;
}


bool ppm_colour_equals(colour_t c1, colour_t c2) {
	return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}


image_t** ppm_image_split(image_t *src, size_t nx, size_t ny) {
	size_t into_w = src->w / nx;
	size_t into_h = src->h / ny;
	ASSERT(into_w * nx == src->w);
	ASSERT(into_h * ny == src->h);
	// make sure that src->w is a multiple of w and src->h is multiple of h
	// so that the image can actually be split up equally

	image_t **into = malloc(sizeof(image_t*) * nx * ny);

	// this just loops over into, initialises the target images and loops
	// over the pixels to copy them over from src
	// naming:
	// ix/iy: x/y position of the current subimages
	// px/py: x/y position of the current pixel
	// idk if this formatting is better readable than indenting everything
	// but 5 levels of indentation is just too disgusting to leave it
	for (size_t iy = 0; iy < ny; iy++)
	for (size_t ix = 0; ix < nx; ix++) {
		size_t cur_img_i = ix + iy * nx;
		into[cur_img_i] = ppm_image_new(into_w, into_h);

		for (size_t py = 0; py < into_h; py++)
		for (size_t px = 0; px < into_w; px++) {
			ppm_image_set_pixel(into[cur_img_i], px, py,
				*ppm_image_get_pixel(src,
					ix * into_w + px, iy * into_h + py));
		}
	}

	return into;
}


void ppm_write_header(FILE *f, size_t w, size_t h) {
	fprintf(f, "P6\n"SIZE_T_FORMAT" "SIZE_T_FORMAT"\n%d\n", w, h, PPM_MAX_COLOUR);
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
		ASSERT_ALWAYS_MSG(!feof(f), "found eof while parsing ppm header");
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
	UNREACHABLE("failed to parse ppm header (too long)");

	header_copy_success:
	int r = sscanf(header_buf, "%2s "SIZE_T_FORMAT" "SIZE_T_FORMAT" %d",
		format, &w, &h, &max_colour);
	ASSERT_ALWAYS_MSG(r == 4, "failed to parse ppm header");
	ASSERT_ALWAYS(!strcmp(format, PPM_FORMAT));
	ASSERT_ALWAYS(max_colour <= PPM_MAX_COLOUR);

	// initialise the image
	ppm_image_init(img, w, h);

	colour_t *buf = malloc(sizeof(colour_t) * w * h);
	fread(buf, sizeof(colour_t), w * h, f);

	for (int px = 0; px < w; px++) {
		for (int py = 0; py < h; py++) {
			const char *raw = (char*) &buf[px + py * w];
			ppm_image_set_pixel(img, px, py,
				(colour_t) {raw[2], raw[1], raw[0]});
			// the colours are ordered differently in the ppm
			// representation and the 3ds internal one
		}
	}

	free(buf);
	
	// TODO working way to assert this:
	// ASSERT_ALWAYS(fgetc(f) == EOF_REAL \
	// 	" (when loading an image file, expected eof)");
}


bool ppm_image_is_in_bounds(image_t *img, size_t x, size_t y) {
	return x < img->w && y < img->h;
}


void ppm_image_set_pixel(image_t *img, size_t x, size_t y, colour_t to) {
	ASSERT(x < img->w);
	ASSERT(y < img->h);
	img->pixels[x * img->h + (img->h - 1 - y)] = to;
	// the 3ds framebuffer grows from bottom left y first
	// in other words it is rotated 90 degrees counter clockwise
}


void ppm_image_fill_rectangle(image_t *img, size_t px, size_t py, size_t sx,
	        size_t sy, colour_t colour) {
	for (size_t x = px; x < px + sx; x++) {
		for (size_t y = py; y < py + sy; y++) {
			ppm_image_set_pixel(img, x, y, colour);
		}
	}
}


colour_t* ppm_image_get_pixel(image_t *img, size_t x, size_t y) {
	ASSERT(x < img->w);
	ASSERT(y < img->h);
	return &img->pixels[x * img->h + (img->h - 1 - y)];
}



void ppm_image_write(image_t *img, FILE *f) {
	ppm_write_header(f, img->w, img->h);
	ppm_image_write_pixels(img, f);
}

void ppm_image_write_pixels(image_t *img, FILE *f) {
	for (size_t y = 0; y < img->h; y++) {
		for (size_t x = 0; x < img->w; x++) {
			colour_t* c = ppm_image_get_pixel(img, x, y);
			fprintf(f, "%c%c%c", c->r, c->g, c->b);
		}
	}
}
