#include "renderer.h"

#include <3ds.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "3ds/gfx.h"
#include "3ds/os.h"
#include "3ds/services/gspgpu.h"
#include "3ds/svc.h"
#include "game.h"
#include "map.h"
#include "maths.h"
#include "ppm.h"
#include "shared.h"
#include "util.h"



void draw_pixel(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res,
	shading_function_t shade, void *shade_params, size_t px, size_t py);


// projects a pixel (x, y) from the output image onto the virtual camera plane 
// fov and view_direction in rad
// w, h: size of the image
// x, y: coordinates of the projected pixel in the image
// d: direction vector of the camera
// c: principle point of the image
vec3_t project_pixel_onto_image_plane(int w, int h, int x, int y, vec3_t c, vec3_t d) {
	return (vec3_t) {c.x + (x - (float) w/2 + 0.5) * d.y,
		c.y - (x - (float) w/2 + 0.5) * d.x,
		y - (float) h/2 + 1};
}


// i: vector origin
// r: vector direction (normalised)
ray_cast_result_t cast_ray(map_t *map, vec3_t i, vec3_t r) {
	ASSERT(r.z == 0);
	// this function should only be used for rays that have z = 0
	ASSERT(is_approx_zero(vec3_length(r) - 1));
	// assert r is normalised

	int r_x_sign = r.x / fabs(r.x);
	int r_y_sign = r.y / fabs(r.y);

	RENDER_DEBUG(printf("casting ray from %f %f %f in dir (%f %f %f)\n",
		i.x, i.y, i.z, r.x, r.y, r.z));

	// travel distance for a full unit in x/y direction
	// called s_w and s_h in the project description
	vec3_t t = {fabs(1/r.x), fabs(1/r.y), 0};
	// we actually do not need to care if r.x is 0, since the 1/r.x will be
	// inf, so s.x will also be inf, therefore never be smaller than s.y and
	//  never be the direction of the next step
	RENDER_DEBUG(printf("t: %f %f %f\n", t.x, t.y, t.z));

	vec3_t in_cell_pos = {fmod1(i.x), fmod1(i.y), 0};
	// the position inside of the cell, so only the fractional part of the
	// position (or fractional part + 1 for numbers < 0), since we want
	// something in the range [0, 1)


	// initialize s (current position in the raycasting algorithm)
	vec3_t s = {0,0,0};

	// depending on if the ray goes to the right or left, take the distance
	// to the right or left side of the cell as the starting value
	// same for y direction
	if (r.x > 0) {
		s.x = (1 - in_cell_pos.x) * t.x;
	} else {
		s.x = in_cell_pos.x * t.x;
	}
	if (r.y > 0) {
		s.y = (1 - in_cell_pos.y) * t.y;
	} else {
		s.y = in_cell_pos.y * t.y;
	}

	// now we can begin with the actual raycasting algorithm
	int last_stepped = s.x < s.y ? DIR_X : DIR_Y;
	// TODO does this initial value make sense?

	ray_cast_result_t res;
	res.hit_cell_tex = CELL_TEX_EMPTY;
	// this remains unchanged if the ray doesnt hit anything
	res.cell_x = i.x < 0 ? (int) i.x - 1 : (int) i.x;
	res.cell_y = i.y < 0 ? (int) i.y - 1 : (int) i.y;
	memset(&res.hit_sprites, 0, sizeof(res.hit_sprites));
	res.ray_direction = r;
	res.ray_origin = i;

	while (map_is_in_bounds(map, res.cell_x, res.cell_y)) {
		RENDER_DEBUG(printf("now with s=(%f %f %f), cell pos = (%d %d)"\
				" and last_stepped_x = %b\n", s.x, s.y, s.z,
				res.cell_x, res.cell_y, last_stepped_x));
		cell_t *hit_cell = map_get_cell(map, res.cell_x, res.cell_y);

		// check for intersection with any props
		// this only checks if the ray goes through the bounding box
		// of the prop
		// it might not actually hit bc it is transparent where it
		// was hit but this will be determined in draw

		for (size_t si = 0; si < MAX_SPRITES_PER_CELL; si++) {
			sprite_t *sprite = &hit_cell->sprites[si];
			if (sprite->tex == SPRITE_TEX_EMPTY) continue;
			
			// FIXME the hit props should be ordered by the distance
			// to the camera, but they arent necessarily if there
			// are multiple in the same cell
			// this could lead to props being draw above props
			// infront of them


			// fprintf(g_log_file, "sprite->pos = " VEC3_FMT ", r = " VEC3_FMT "\n", VEC3_SPLIT(sprite->pos), VEC3_SPLIT(r));
			vec3_t to_sprite = vec3_sub(sprite->pos, i);
			float sprite_centre_distance = vec2_length(to_sprite);
			float t = vec2_dot_product(r, to_sprite);
				// / vec3_dot_product(r, r);
				// |r| = 1 so r.r = 1
			if (t < 0) continue;
			// 	// sprite is in opposite ray direction
			// 	// ie behind the camera

			float angle = acosf(t / sprite_centre_distance);
			// see vec2_angle and |r| = 1

			float sprite_centre_intersection_distance =
				tan(angle) * sprite_centre_distance;

			int side = signf(r.x * to_sprite.y - to_sprite.x * r.y);
			// whether the hit happened on the right side or the
			// left side of the ray
			// sign of the determinant of (r|to_prop)
			// from stack overflow question 13221873

			if (sprite_centre_intersection_distance > sprite->width) continue;
				// ray doesnt hit

			u8 tex_pos_x = (1 - sprite->width +
					sprite_centre_intersection_distance * side)
				* (float) TEX_DIMENSIONS;

			for (size_t sj = 0; sj < MAX_SPRITES_PER_CELL; sj++) {
				if (res.hit_sprites[sj].sprite != NULL) continue;
				
				float hit_distance = sprite_centre_distance / cos(angle);
				
				res.hit_sprites[sj].sprite = sprite;
				res.hit_sprites[sj].pos = vec3_add(i, vec3_mul_scalar(hit_distance, r));
				res.hit_sprites[sj].tex_pos_x = tex_pos_x;
				res.hit_sprites[sj].distance = hit_distance;
				break;
				
			}
		}
		
		
		if (hit_cell->tex != CELL_TEX_EMPTY) {
			res.hit_cell_tex = hit_cell->tex;
			break;
		}
		
		// now step
		if (s.x < s.y) {
			s.x += t.x;
			res.cell_x += r_x_sign;
			last_stepped = DIR_X;
		} else {
			s.y += t.y;
			res.cell_y += r_y_sign;
			last_stepped = DIR_Y;
		}
	}

	res.wall_orientation = 1 - last_stepped;
	// flip this
	if (last_stepped == DIR_X) {
		res.pos = vec3_add(i, vec3_mul_scalar(s.x - t.x, r));
	} else {
		res.pos = vec3_add(i, vec3_mul_scalar(s.y - t.y, r));
	}

	// (comment from renderer.h)
	// calculate the x coordinate of the wall texture that was hit
	// this doesnt really belong here but due to the way rendering is
	// implemented this can be calculated once per row in cast_ray instead
	// of being calculated for every pixel in draw_pixel
	// TODO are the textures now flipped?
	if (res.hit_cell_tex != CELL_TEX_EMPTY) {
		switch (res.wall_orientation) {
		break; case DIR_X:
			res.tex_pos_x = (TEX_DIMENSIONS - 1) -
				(u8) ((res.pos.x - (s8) res.pos.x)
					* (float) TEX_DIMENSIONS);
			if (r.y < 0) res.tex_pos_x = (TEX_DIMENSIONS - 1)
				        - res.tex_pos_x;
		break; case DIR_Y:
			res.tex_pos_x = (TEX_DIMENSIONS - 1) -
				(u8) ((res.pos.y - (s8) res.pos.y)
					* (float) TEX_DIMENSIONS);
			if (r.x > 0) res.tex_pos_x = (TEX_DIMENSIONS - 1)
			        	- res.tex_pos_x;
		};
	}

	return res;
}


 void render(image_t *img, map_t *map, float px, float py, float fov,
		float rotation, tex_atlas_t *tex_atlas,
		shading_function_t shade, void *shade_params, size_t scale) {
 	
	// TODO better implementation of scaling
	// i think it would be faster to loop over all x / y not just multiples
	// of scale, and if they are not a multiple of scale just copy the
	// previous pixel (y case) or column (x case)

 	
	ASSERT(0 < fov && fov < PI);

	RENDER_DEBUG(printf("w: %d, h: %d, px: %f, py: %f fov: %f, rot: %f\n",
			w,h,px,py,fov,rotation));

	// player position and aparture
	vec3_t p = {px, py, CAMERA_HEIGHT};

	// direction vector
	vec3_t d = {cosf(rotation), sinf(rotation), 0};

	// focal distance
	float f = img->w / (2 * tan(fov/2));

	// principle point of the image
	vec3_t c = vec3_sub(p, vec3_mul_scalar(f, d));

	RENDER_DEBUG(printf("player pos: (%f %f %f), focal distance: %f, "\
			"direction: (%f %f %f), priciple point: (%f %f %f)\n",
			p.x, p.y, p.z, f, d.x, d.y, d.z, c.x, c.y, c.z));

	const vec3_t i_top_left =
		project_pixel_onto_image_plane(img->w, img->h, 0, 0, c, d);
	const vec3_t i_x_delta = vec3_sub(
		project_pixel_onto_image_plane(img->w, img->h, 1, 0, c, d),
		i_top_left);
	const vec3_t i_y_delta = vec3_sub(
		project_pixel_onto_image_plane(img->w, img->h, 0, scale, c, d),
		i_top_left);
	// the distance between the projected pixels is always the same, so
	// we dont have to recompute it every time

	// projection of the currently drawn pixel
	vec3_t i = i_top_left;

	for (size_t x = 0; x < img->w; x += scale) {
		// first cast a ray parallel to the floor from z=0
		// this checks if there is even a wall on that column at all

		i = vec3_add(i_top_left, vec3_mul_scalar(x, i_x_delta));

		vec3_t horizontal_r = vec3_sub(p, i);
		horizontal_r.z = 0;
		horizontal_r = vec3_normalised(horizontal_r);

		ray_cast_result_t rc_res = cast_ray(map, p, horizontal_r);
		// the project description says to cast the ray from i, but we
		// can cast if from p instead since the ray passes through p
		// anyways and we dont want to see things between i and p. this
		// also makes sure that the ray always starts in bounds so there
		// is no need to handle the other case

		// NOTE if i change this to i for some reason, remember to also
		// change it in the next line (definition of wall_distance)

		float wall_distance = vec3_length(
		 vec3_sub(rc_res.pos, p));
		RENDER_DEBUG(printf("wall distance: %f\n", wall_distance));

		// now loop over all rows (y pixels) in this column
		vec3_t r = horizontal_r;
		// new direction for the z-angled rays
		for (size_t y = 0; y < img->h; y += scale) {
			// we dont have to cast another ray here.
			// the idea here is that we represent r as
			// {horizontal_r.x, horizontal_r.y, some z}
			// this must be possible because they both point in the
			// same direction on the x-y plane
			// then if we do that, we know that wall_distance * r
			// hits the same wall in the same column as
			// wall_distance * horizontal_r, in other words the
			// intersection point is p + wall_distance * r,
			// including for the z coordinate

			i = vec3_add(i, i_y_delta);
			r = vec3_sub(p, i);
			// no need to normalise this here (see next comment)
			r = vec3_mul_scalar(horizontal_r.x / r.x, r);
			// (this comment is probably incorrcect)
			// this makes sure that r.x == horizontal_r.x and
			// r.y == horizontal_r.y (as mentioned above).
			// since r points in the same x and y direction as
			// original_r, original_r.x / r.x == original_r.y / r.y
			// so it doesnt matter which one we take here

			// FIXME the magnitude of r in not constant (even with
			// normalising) its only like 1.04 so its not too bad
			// i think its because r.z != 0 and horizontal_r.z == 0
			// is there a wayy to precompute some factor?

			// we only need to compute the z coordinate of the
			// intersection, not x and y since they stay the same
			rc_res.pos.z = p.z + r.z * wall_distance;
			rc_res.ray_direction.z = r.z;

			// update z positions of prop intersections
			for (int i = 0; i < MAX_SPRITES_PER_CELL; i++) {
				if (rc_res.hit_sprites[i].sprite == NULL) break;
				rc_res.hit_sprites[i].pos.z =
					r.z * rc_res.hit_sprites[i].distance
					+ CAMERA_HEIGHT;
			}

			// we can leave the other values is rc_res as they dont
			// change
			draw_pixel(img, tex_atlas, &rc_res, shade,
				shade_params, x, y);
			#ifdef OPTIMIZE_DOUBLE_SCALE
			colour_t *colour = ppm_image_get_pixel(img, x, y);
			ppm_image_set_pixel(img, x + 1, y, *colour);
			ppm_image_set_pixel(img, x, y + 1, *colour);
			ppm_image_set_pixel(img, x + 1, y + 1, *colour);
			#else
			if (scale != 1)
				ppm_image_fill_rectangle(img, x, y, scale,
					scale, *ppm_image_get_pixel(img, x, y));
			#endif
                }
	}
}


void draw_pixel(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res,
		shading_function_t shade, void *shade_params, size_t px
		, size_t py) {
	for (size_t i = 0; i < MAX_SPRITES_PER_CELL; i++) {
		// we assume that the props are ordered by distance from the
		// camera, which is not necessarily the case when there are
		// multiple props in the same cell
		// FIXME

		sprite_hit_t *sprite_hit = &rc_res->hit_sprites[i];
		if (sprite_hit->sprite == NULL) break;

		if (sprite_hit->pos.z < 0 || 1 <  sprite_hit->pos.z) break;

		u8 ty = (TEX_DIMENSIONS - 1)
			- sprite_hit->pos.z * (float) TEX_DIMENSIONS;
		
		colour_t *tex_pix =  ppm_image_get_pixel(
				ta->tex[sprite_hit->sprite->tex],
				sprite_hit->tex_pos_x, ty);

		if (ppm_colour_equals(*tex_pix, COLOUR_TRANSPARENT)) continue;
		colour_t *pix = ppm_image_get_pixel(img, px, py);
		*pix = *tex_pix;
		shade(px, py, pix, SHADE_FLAG_SPRITE, shade_params);
		
		return;
	}

	
	if (rc_res->hit_cell_tex == CELL_TEX_EMPTY || rc_res->pos.z < 0.0001 ||
			rc_res->pos.z > 0.9999) {
		// drawing floor / ceil
		if (py * 2 - 1 == img->h) {
			// drawing to the middle row of an image with odd height
			ppm_image_set_pixel(img, px, py, COLOUR_BLACK);
			return;
		}

		// first we need to actuall get the intersection with the floor
		// / the ceiling

		vec3_t fc_intersection = vec3_add(rc_res->ray_origin,
			vec3_mul_scalar(
				fabs(0.5 / rc_res->ray_direction.z),
				rc_res->ray_direction));

		// get the intersection inside the cell so in [0, 1)
		float in_cell_x = fmod1(fc_intersection.x);
		float in_cell_y = fmod1(fc_intersection.y);

		u8 ty = (size_t) (in_cell_y * TEX_DIMENSIONS);
		u8 tx = (size_t) (in_cell_x * TEX_DIMENSIONS);

		ASSERT(ty < TEX_DIMENSIONS);
		ASSERT(tx < TEX_DIMENSIONS);

		size_t tex_index = is_approx_zero(fc_intersection.z) ? 0 : 1;

		ASSERT(tex_index < ta->count);

		colour_t *pixel = ppm_image_get_pixel(img, px, py);
		*pixel = *ppm_image_get_pixel(ta->tex[tex_index], tx, ty);
		shade(px, py, pixel, SHADE_FLAG_FLOOR_CEIL, shade_params);
	} else {
		// drawing wall
		u8 tx, ty;
		//texture coordinates
		ty = (TEX_DIMENSIONS - 1)
			- (size_t) (rc_res->pos.z * TEX_DIMENSIONS);
		tx = rc_res->tex_pos_x;

		ASSERT(rc_res->hit_cell_tex < ta->count);

		colour_t *pixel = ppm_image_get_pixel(img, px, py);

		*pixel = *ppm_image_get_pixel(
			ta->tex[rc_res->hit_cell_tex], tx, ty);

		shade(px, py, pixel, rc_res->wall_orientation == DIR_X ?
			SHADE_FLAG_WALL_X : SHADE_FLAG_WALL_Y, shade_params);
	}
}


// void shade_default(size_t px, size_t py, colour_t *pixel, int flags,
//		void *params) {
// 	if (flags & SHADE_FLAG_WALL_X) {
// 		pixel->r = (pixel->r >> 1) & 0x7f;
// 		pixel->g = (pixel->g >> 1) & 0x7f;
// 		pixel->b = (pixel->b >> 1) & 0x7f;
// 	}
// }


void shade_dim(size_t px, size_t py, colour_t *pixel, int flags, void *by) {
	pixel->r = pixel->r / *(u8 *) by;
	pixel->g = pixel->g / *(u8 *) by;
	pixel->b = pixel->b / *(u8 *) by;
}


void shade_blink(size_t px, size_t py, colour_t *pixel, int flags, void *params) {
	static bool on = true;
	static float next_switch_ms = 0;

	if (px == 0 && py == 0) {
		float now = svcGetSystemTick() / CPU_TICKS_PER_MSEC;
		check:
		if (now < next_switch_ms) goto shade;
		if (on) {
			on = false;
			next_switch_ms = now +
				BLINK_INTERVAL_MS * (1 - *(float *) params);
			goto check;
		} else {
			on = true;
			next_switch_ms = now +
				BLINK_INTERVAL_MS * *(float *) params;
			goto check;
		}
	}

	shade:
	if (on) {
		pixel->r = pixel->r / 4;
		pixel->g = pixel->g / 4;
		pixel->b = pixel->b / 4;
	} else {
		if (flags & SHADE_FLAG_SPRITE) {
			u8 dim_factor = 8;
			shade_dim(px, py, pixel, flags, (void *) &dim_factor);
		} else 
			*pixel = COLOUR_BLACK;
	}
}


void tex_atlas_load(tex_atlas_t *tex_atlas, FILE *f) {
	image_t img;
	ppm_image_load(&img, f);

	size_t nx = img.w / TEX_DIMENSIONS;
	size_t ny = img.h / TEX_DIMENSIONS;

	ASSERT(nx * TEX_DIMENSIONS == img.w);
	ASSERT(ny * TEX_DIMENSIONS == img.h);

	tex_atlas->tex = ppm_image_split(&img, nx, ny);
	// this implicitly confirms that nx and ny are non-zero
	ppm_image_free(&img);

	tex_atlas->nx = nx;
	tex_atlas->ny = ny;

	tex_atlas->count = nx * ny;
}

void tex_atlas_free(tex_atlas_t *ta) {
	for (size_t i = 0; i < ta->nx * ta->ny; i++) {
		ppm_image_free(ta->tex[i]);
		free(ta->tex[i]);
	}
	free(ta->tex);
	ta->tex = NULL;
}



void render_frame_begin() {
	gfxSwapBuffers();
	gspWaitForVBlank();
}


void render_top_screen() {
	float fov = deg_to_rad(69);
	// rendering
	image_t fb;
	fb.w = 400;
	fb.h = 240;
	fb.pixels = (colour_t*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);


	// calculate blink frequency

	float distance = vec3_length(
		vec3_sub(g_game.player.pos, g_game.creature.pos));

	float ratio = fminf(1, distance / BLINK_MAX_DISTANCE);
	// ratio *= ratio;
	
	render(&fb, &g_game.map, g_game.player.pos.x, g_game.player.pos.y, fov, 
		// g_game.player.rotation, &g_game.ta, shade_dark, (void *) &dim_factor,
		g_game.player.rotation, &g_game.ta, shade_blink, (void *) &ratio,
		2);
}


void render_image(image_t *img) {
	colour_t *fb = (colour_t *) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(fb, img->pixels, GSP_SCREEN_WIDTH * GSP_SCREEN_HEIGHT_TOP
			* sizeof(img->pixels[0]));
}


void render_frame_end() {
	gfxFlushBuffers();
}

// dont think ill need this but keep it just in case
/* void draw_untextured(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res,
		size_t px, size_t py) {
	if (rc_res->hit == CELL_EMPTY || rc_res->position.z < 0.0001 ||
			rc_res->position.z > 0.9999) {
		ppm_image_set_pixel(img, px, py, COLOUR_BLACK);
		return;
	}
	switch (rc_res->wall_orientation) {
		case DIR_X:
			ppm_image_set_pixel(img, px, py, COLOUR_GREEN);
			break;
		case DIR_Y:
			ppm_image_set_pixel(img, px, py, COLOUR_RED);
			break;
	}
} */

