#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include "map.h"
#include "maths.h"
#include "ppm.h"


#define CAMERA_OFFSET (vec3_t) {0,0,0.5}

enum {
	TEX_TYPE_FLOOR = -3,
	TEX_TYPE_CEIL = -2,
	TEX_TYPE_EMPTY = -1,
	// 0 to 9 are reserved for textured walls
	TEX_TYPE_NOTEX_WALL_X = 10,
	TEX_TYPE_NOTEX_WALL_Y = 11,
};

enum {
	DIR_X = 0,
	DIR_Y = 1,
};


typedef int8_t tex_type_t;


typedef struct {
	tex_type_t *pixels;
	int w, h;
} render_buf_t;
// internal representation of the rendered view containing TEX_TYPE_... stuff


typedef struct {
	cell_t hit;
	vec3_t position;
	int wall_orientation;
} ray_cast_result_t;


typedef struct {
	image_t **tex;
	size_t count;
	size_t w, h;
	// image dimensions
} tex_atlas_t;


void tex_atlas_load(tex_atlas_t* tex_atlas, FILE *f, size_t nx, size_t ny);


render_buf_t *render_buf_new(size_t w, size_t h);
void render_buf_init(render_buf_t *buf, size_t w, size_t h);


// buf can be NULL, then a new one is allocated
// but when calling this multiple times it makes sense to reuse the same one
// to prevent unnecessary allocations
//
// void* is only temporary until i have implemented a texture atlas
void render(FILE* out, render_buf_t *buf, map_t *map, void *tex_atlas, int w, int h, double px,
		double py, double fov, double rotation);
#endif
