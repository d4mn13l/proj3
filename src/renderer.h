#ifndef RENDERER_H
#define RENDERER_H

#include <stddef.h>
#include <stdint.h>
#include "map.h"
#include "maths.h"
#include "ppm.h"


#define CAMERA_OFFSET (vec3_t) {0,0,0.5}

enum {
	DIR_X = 0,
	DIR_Y = 1,
};


enum {
	SHADE_FLAG_WALL_X = 1,
	SHADE_FLAG_WALL_Y = 2,
	SHADE_FLAG_FLOOR_CEIL = 4,
	SHADE_FLAG_PROP = 8,
};

typedef struct {
	vec3_t ray_origin;
	vec3_t ray_direction;
	tex_t hit_cell_tex;
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


typedef void(*shading_function_t)(size_t x, size_t y, colour_t*, int flags,
        void *params);


void shade_default(size_t x, size_t y, colour_t *pixel, int flags, void *params);
// doesnt use params
void shade_dark(size_t x, size_t y, colour_t *pixel, int flags, void *by);
// by to be uint8_t* and darkens every pixel by it
void shade_blink(size_t x, size_t y, colour_t *pixel, int flags, void *params);
// expects params to be the current time as float*
#define SHADE_BLINK_ON_TIME 0.4
#define SHADE_BLINK_OFF_TIME 0.7


void tex_atlas_load(tex_atlas_t* ta, FILE *f, size_t nx, size_t ny);
void tex_atlas_free(tex_atlas_t* ta);


// renders the map viewed from (px, py), with rotation and fov into img
// using draw_function
// expects rotation and fov in radians
void render(image_t *img, map_t *map, float px, float py, float fov,
		float rotation, tex_atlas_t *tex_atlas,
		shading_function_t shade, void *shade_params);
#endif
