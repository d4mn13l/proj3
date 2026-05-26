#include "renderer.h"

#include <math.h>
#include <stdbool.h>

#include "map.h"
#include "maths.h"
#include "ppm.h"
#include "util.h"



void draw_untextured(FILE*, render_buf_t*);


// projects a pixel (x, y) from the output image onto the virtual camera plane 
// fov and view_direction in rad
// w, h: size of the image
// x, y: coordinates of the projected pixel in the image
// d: direction vector of the camera
// c: principle point of the image
vec3_t project_pixel_onto_image_plane(int w, int h, int x, int y, vec3_t c, vec3_t d) {
	return (vec3_t) {c.x + (x - (double) w/2 + 0.5) * d.y,
		c.y - (x - (double) w/2 + 0.5) * d.x,
		y - (double) h/2 + 1};
}


// i: vector origin
// r: vector direction
ray_cast_result_t cast_ray(map_t *map, vec3_t i, vec3_t r) {
	ASSERT(r.z == 0, __LINE__, __FILE__);
	// this function should only be used for rays that have z = 0

	r = vec3_normalised(r);
	int r_x_sign = r.x / fabs(r.x);
	int r_y_sign = r.y / fabs(r.y);

	RENDER_DEBUG(printf("casting ray from %f %f %f in direction (%f %f %f)\n",
	       i.x, i.y, i.z, r.x, r.y, r.z));

	// travel distance for a full unit in x/y direction
	// called s_w and s_h in the project description
	vec3_t t = {fabs(1/r.x), fabs(1/r.y), 0};
	// we actually do not need to care if r.x is 0, since the 1/r.x will be inf,
	// so s.x will also be inf, therefore never be smaller than s.y and never be
	// the direction of the next step
	RENDER_DEBUG(printf("t: %f %f %f\n", t.x, t.y, t.z));

	vec3_t in_cell_pos = {i.x - (int) i.x, i.y - (int) i.y, 0};
	// the position inside of the cell, so only the decimal part of the position 
	// we want something in the range [0, 1), however this calculation  can 
	// result in negative values if the original position is also negative, 
	// but always in the range (-1, 1)
	if (in_cell_pos.x < 0) in_cell_pos.x += 1;
	if (in_cell_pos.y < 0) in_cell_pos.y += 1;
	// so this brings the value in the desired range

	int cell_pos_x = i.x < 0 ? (int) i.x - 1 : (int) i.x;
	int cell_pos_y = i.y < 0 ? (int) i.y - 1 : (int) i.y;

	// initialize s (current position in the raycasting algorithm)
	vec3_t s = {0,0,0};

	// depending on if the ray goes to the right or left, take the distance to
	// the right or left side of the cell as the starting value
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
	res.hit = CELL_EMPTY;
	// this remains unchanged if the ray doesnt hit anything

	while (map_is_in_bounds(map, cell_pos_x, cell_pos_y)) {
		RENDER_DEBUG(printf("now with s=(%f %f %f), cell pos = (%d %d) and last_stepped_x = %b\n",
			s.x, s.y, s.z, cell_pos_x, cell_pos_y, last_stepped_x));
		cell_t hit_cell = map_get_cell(map, cell_pos_x, cell_pos_y);

		if (hit_cell != CELL_EMPTY) {
			res.hit = hit_cell;
			break;
		}
		
		// now step
		if (s.x < s.y) {
			s.x += t.x;
			cell_pos_x += r_x_sign;
			last_stepped = DIR_X;
		} else {
			s.y += t.y;
			cell_pos_y += r_y_sign;
			last_stepped = DIR_Y;
		}
	}

	res.wall_orientation = 1 - last_stepped;
	// flip this because uhh idk
	// theres probably another flip somewhere else that cancels this out
	if (last_stepped == DIR_X) {
		res.position = vec3_add(i, vec3_mul_scalar(s.x - t.x, r));
	} else {
		res.position = vec3_add(i, vec3_mul_scalar(s.y - t.y, r));
	}

	return res;
}


void render(FILE *out, render_buf_t *buf, map_t *map, void *tex_atlas, int w, int h, double px, 
		double py, double fov, double rotation) {
	ASSERT(0 < fov && fov < 180, __LINE__, __FILE__);
	
	bool free_render_buf = false;
	if (buf == NULL) {
		buf = render_buf_new(w, h);
		free_render_buf = true;
	}

	
	// convert fov and rotation to radians
	fov = deg_to_rad(fov);
	rotation = deg_to_rad(rotation);

	RENDER_DEBUG(printf("w: %d, h: %d, px: %f, py: %f fov: %f, rot: %f\n", w,h,px,py,fov,rotation));

	// player position and aparture
	vec3_t p = {px, py, 0.5};

	// direction vector
	vec3_t d = {cos(rotation), sin(rotation), 0};

	// focal distance
	double f = w / (2 * tan(fov/2));

	// principle point of the image
	vec3_t c = vec3_sub(p, vec3_mul_scalar(f, d));

	RENDER_DEBUG(printf("player pos: (%f %f %f), focal distance: %f, direction: (%f %f %f), priciple point: (%f %f %f)\n",
		p.x, p.y, p.z, f, d.x, d.y, d.z, c.x, c.y, c.z));

	for (int x = 0; x < w; x++) {
		// first cast a ray parallel to the floor from z=0
		// this checks if there is even a wall on that column at all

		vec3_t i = project_pixel_onto_image_plane(w, h, x, 0, c, d);
		i.z = 0.5;
		vec3_t r = vec3_sub(p, i);
		r.z = 0;

		ray_cast_result_t rc_res = cast_ray(map, p, r);
		// the project description says to cast the ray from i, but we can cast if
		// from p instead since the ray passes through p anyways and we dont want
		// to see things between i and p. this also makes sure that the ray always
		// starts in bounds so there is no need to handle the other case

		// NOTE if i change this to i for some reason, remember to also change it in
		// the next line (definition of wall_distance)

		double wall_distance = vec3_magnitude(vec3_sub(rc_res.position, vec3_sub(p, CAMERA_OFFSET)));
		RENDER_DEBUG(printf("wall distance: %f\n", wall_distance));

		tex_type_t tex;
		if (tex_atlas != NULL) {
			switch (rc_res.hit) {
				case CELL_EMPTY:
					tex = TEX_TYPE_EMPTY;
					break;
				case CELL_UNTEXTURED_WALL:
					if (tex_atlas) tex = 2;
					break;
					tex = TEX_TYPE_NOTEX_WALL_X + rc_res.wall_orientation;
					// if rc_res.wall_orientation is DIR_X, then this will result in
					// TEX_UNTEXTURED_WALL_X. a bit hacky but its 1 less if statement
					break;
				default:
					// assume this is in [0,9], so a valid texture atlas index
					// there shouldnt be any other possible value
					tex = rc_res.hit;
			};
		} else {
			switch (rc_res.hit) {
				case CELL_EMPTY:
					tex = TEX_TYPE_EMPTY;
					break;
				default:
					tex = TEX_TYPE_NOTEX_WALL_X + rc_res.wall_orientation;
					// works for the same reason as above
			}
		}


		if (tex == TEX_TYPE_EMPTY) {
			// FIXME what to put in middle row if h is odd?
			for (int y = 0; y < h/2; y++) {
				buf->pixels[x + y * w] = TEX_TYPE_CEIL;
			}
			for (int y = h/2; y < h; y++) {
				buf->pixels[x + y * w] = TEX_TYPE_FLOOR;
			}
			continue;
		}

		// in this case the horizontal ray does hit
		// TODO binary search the top/bottom of the wall or directly compute it?

		for (int y = 0; y < h; y++) {
			i = project_pixel_onto_image_plane(w, h, x, y, c, d);
			r = vec3_normalised(vec3_sub(p, i));

			double intersection_z = p.z + r.z * wall_distance;

			if (intersection_z < 0.0001) {
				buf->pixels[x + y * w] = TEX_TYPE_FLOOR;
			} else if (intersection_z > 1.0001) {
				buf->pixels[x + y * w] = TEX_TYPE_FLOOR;
			} else {
				buf->pixels[x + y * w] = tex;
			}
		}
	}

	ppm_write_header(out, w, h);
	draw_untextured(out, buf);

	if (free_render_buf) free(buf);
}


void draw_untextured(FILE *f, render_buf_t *buf) {
	for (int i = 0; i < buf->w * buf->h; i++) {
		switch (buf->pixels[i]) {
			case TEX_TYPE_NOTEX_WALL_X:
				ppm_write_colour(f, COLOUR_GREEN);
				break;
			case TEX_TYPE_NOTEX_WALL_Y:
				ppm_write_colour(f, COLOUR_RED);
				break;
			case TEX_TYPE_CEIL:
			case TEX_TYPE_FLOOR:
			case TEX_TYPE_EMPTY:
				ppm_write_colour(f, COLOUR_BLACK);
				break;
			default:
				UNREACHABLE(__LINE__, __FILE__, "unexpected tex_type in draw_untextured");
		}
	}
}


void tex_atlas_load(tex_atlas_t *tex_atlas, FILE *f, size_t nx, size_t ny) {
	image_t img;
	ppm_image_load(&img, f);

	tex_atlas->tex = ppm_image_split(&img, nx, ny);
	ppm_image_free(&img);

	tex_atlas->w = nx;
	tex_atlas->h = ny;
	tex_atlas->count = nx * ny;
}



render_buf_t* render_buf_new(size_t w, size_t h) {
	render_buf_t *buf = malloc(sizeof(render_buf_t));
	ASSERT(buf != NULL, __LINE__, __FILE__);
	render_buf_init(buf, w, h);
	return buf;
}


void render_buf_init(render_buf_t *buf, size_t w, size_t h) {
	buf->w = w;
	buf->h = h;
	buf->pixels = malloc(w * h * sizeof(int8_t));
	ASSERT(buf->pixels != NULL, __LINE__, __FILE__);
}

