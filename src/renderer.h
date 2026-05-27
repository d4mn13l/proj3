#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include "map.h"
#include "maths.h"
#include "ppm.h"


#define CAMERA_OFFSET (vec3_t) {0,0,0.5}

enum {
	DIR_X = 0,
	DIR_Y = 1,
};


typedef struct {
	cell_t hit;
	size_t cell_x, cell_y;
	vec3_t position;
	int wall_orientation;
} ray_cast_result_t;


typedef struct {
	image_t **tex;
	size_t count;
	size_t w, h;
	// image dimensions
} tex_atlas_t;


typedef void(*draw_function_t)(image_t*, tex_atlas_t*,  ray_cast_result_t*,
                		int px, int py);
// responsible for drawing a pixel at (px, py) into the image


// px, py: position in the image
void draw_untextured(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res, int px, int py);


void tex_atlas_load(tex_atlas_t* tex_atlas, FILE *f, size_t nx, size_t ny);

// renders the map viewed from (px, py), with rotation and fov into img
// using draw_function
void render(FILE *out, map_t *map, int w, int h, double px, double py,
		double fov, double rotation, draw_function_t draw,
		tex_atlas_t *tex_atlas, image_t *img);
#endif
