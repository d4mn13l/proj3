#ifndef RENDERER_H
#define RENDERER_H

#include <stddef.h>
#include <stdint.h>
#include "map.h"
#include "maths.h"
#include "ppm.h"


#define CAMERA_OFFSET (vec3_t) {0,0,0.5}

enum {
	DRAW_FLAGS_EMPTY = 0,
	DRAW_FLAG_DO_SHADING = 1,
	DRAW_FLAG_RENDER_FLOOR_CEIL = 2,
};

enum {
	DIR_X = 0,
	DIR_Y = 1,
};

typedef struct {
	vec3_t ray_origin;
	vec3_t ray_direction;
	cell_t hit;
	size_t cell_x, cell_y;
	vec3_t position;
	int wall_orientation;
} ray_cast_result_t;


typedef struct {
	image_t **tex;
	size_t count;
	size_t nx, ny;
	size_t w, h;
	// image dimensions
} tex_atlas_t;


typedef void(*draw_function_t)(image_t*, tex_atlas_t*,  ray_cast_result_t*,
		int flags, size_t px, size_t py);
// responsible for drawing a pixel at (px, py) into the image


// px, py: position in the image
void draw_untextured(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res,
		int flags, size_t px, size_t py);
void draw_textured(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res,
		int flags, size_t px, size_t py);



void tex_atlas_load(tex_atlas_t* ta, FILE *f, size_t nx, size_t ny);
void tex_atlas_free(tex_atlas_t* ta);


// renders the map viewed from (px, py), with rotation and fov into img
// using draw_function
// expects rotation and fov in radians
void render(image_t *img, map_t *map, double px, double py,
		double fov, double rotation, draw_function_t draw,
		tex_atlas_t *tex_atlas, int draw_flags);
#endif
