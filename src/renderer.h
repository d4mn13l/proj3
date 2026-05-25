#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include "map.h"


#define CAMERA_OFFSET (vec3_t) {0,0,0.5}

enum {
	TEX_FLOOR = -3,
	TEX_CEIL = -2,
	TEX_EMPTY = -1,
	// 0 to 9 are reserved for textured walls
	TEX_UNTEXTURED_WALL_X = 10,
	TEX_UNTEXTURED_WALL_Y = 11,
};

typedef int8_t texture_t;


typedef struct {
	texture_t *pixels;
	int w, h;
} render_buf_t;
// internal representation of the rendered view containing TEXTURE_... stuff

render_buf_t *render_buf_new(size_t w, size_t h);
void render_buf_init(render_buf_t *buf, size_t w, size_t h);


// buf can be NULL, then a new one is allocated
// but when calling this multiple times it makes sense to reuse the same one
// to prevent unnecessary allocations
void render(FILE* out, render_buf_t *buf, map_t *map, int w, int h, double px,
		double py, double fov, double rotation);
#endif
