#ifndef RENDERER_H
#define RENDERER_H

#include <stddef.h>
#include <stdint.h>
#include "map.h"
#include "maths.h"
#include "ppm.h"


#define CAMERA_HEIGHT 0.5
#define CAMERA_OFFSET (vec3_t) {0, 0, CAMERA_HEIGHT}
#define TEX_DIMENSIONS 64

enum {
	DIR_X = 0,
	DIR_Y = 1,
};


enum {
	SHADE_FLAG_WALL_X = 1,
	SHADE_FLAG_WALL_Y = 2,
	SHADE_FLAG_FLOOR_CEIL = 4,
	SHADE_FLAG_SPRITE = 8,
};


typedef struct {
	sprite_t *sprite;
	u8 tex_pos_x;
	vec3_t pos;
	float distance;
	// distance from the ray origin to the intersection point
} sprite_hit_t;


typedef struct {
	vec3_t ray_origin;
	vec3_t ray_direction;
	size_t cell_x, cell_y;
	vec3_t pos;
	tex_t hit_cell_tex;
	u8 wall_orientation;
	u8 tex_pos_x;
	// the x coordinate of the wall texture that was hit
	// this doesnt really belong here but due to the way rendering is
	// implemented this can be calculated once per row in cast_ray instead
	// of being calculated for every pixel in draw_pixel
	sprite_hit_t hit_sprites[MAX_SPRITES_PER_CELL];
	// use MAX_SPRITES_PER_CELL here bc im too lazy to make another macro
	// but i think its fair to assume a ray will not hit more sprites
	// than that
} ray_cast_result_t;


typedef struct {
	image_t **tex;
	// dimension of the images is always TEX_DIMENSION
	size_t count;
	size_t nx, ny;
} tex_atlas_t;


typedef void(*shading_function_t)(size_t px, size_t py, colour_t*, int flags,
	void *params);


// void shade_default(size_t px, size_t py, colour_t *pixel, int flags,
	// void *params);
// doesnt use params
void shade_dim(size_t px, size_t py, colour_t *pixel, int flags, void *by);
// by to be uint8_t* and darkens every pixel by it (by dividing)
void shade_blink(size_t px, size_t py, colour_t *pixel, int flags,
	void *params);
// expects param to be the ratio between off and on time relative to
// BLINK_INTERVAL_MS in range [0, 1], such that the next off time is
// BLINK_INTERVAL_MS * (1 - ratio) and on time is BLINK_INTERVAL_MS * ratio
// the inverval will only be updated when turning on/off


void tex_atlas_load(tex_atlas_t* ta, FILE *f);
void tex_atlas_free(tex_atlas_t* ta);


// TODO should these even take map as an argument or take the map from g_game?
// renders the map viewed from (px, py), with rotation and fov into img
// using draw_function
// expects rotation and fov in radians
void render(image_t *img, map_t *map, float px, float py, float fov,
		float rotation, tex_atlas_t *tex_atlas,
		shading_function_t shade, void *shade_params, size_t scale);

ray_cast_result_t cast_ray(map_t *map, vec3_t i, vec3_t r);
// expose this to misuse this for line of sight check in creature.c



void render_frame_begin();
void render_top_screen();
void render_frame_end();


#endif
