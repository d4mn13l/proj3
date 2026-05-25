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
texture_t cast_ray(map_t *map, vec3_t i, vec3_t r, vec3_t *out_intersection) {
	ASSERT(r.z == 0, __LINE__, __FILE__);
	// this function should only be used for rays that have z = 0
	// should it??

	ASSERT(r.x != 0, __LINE__, __FILE__" (did u try rendering with an odd width)");
	ASSERT(r.y != 0, __LINE__, __FILE__" (did u try rendering with an odd height)");
	// idk if this can happen, but it will break the calculation of t
	
	r = vec3_normalised(r);
	int r_x_sign = r.x / fabs(r.x);
	int r_y_sign = r.y / fabs(r.y);

	RENDER_DEBUG(printf("casting ray from %f %f %f in direction (%f %f %f)\n",
	       i.x, i.y, i.z, r.x, r.y, r.z));

	// travel distance for a full unit in x/y direction
	// called s_w and s_h in the project description
	vec3_t t = {fabs(1/r.x), fabs(1/r.y), 0};
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
	bool last_stepped_x = s.x < s.y;
	// TODO does this initial value make sense?

	// first make sure the position is actually in bounds since the starting
	// point is on the virtual camera plane which might be out of bounds
//	printf("trying to enter bounds\n");
	// while (!map_is_in_bounds(map, cell_pos_x, cell_pos_y)) {
	// 	printf("step\n");
	// 	// we use the same stepping algorithm as in the actual raycasting and we can 
	// 	// modify s since nothing will be hit anyways since were out of bounds
	// 	printf("cell pos %d %d, s=(%f %f %f)\n", cell_pos_x, cell_pos_y, s.x, s.y, s.z);
	// 	if (s.x < s.y) {
	// 		s.x += t.x;
	// 		cell_pos_x += r_x_sign;
	// 		last_stepped_x = true;
	// 	} else {
	// 		s.y += t.y;
	// 		cell_pos_y += r_y_sign;
	// 		last_stepped_x = false;
	// 	}
	// }
	// printf("entered bounds\n");
	// this is not needed rn as the player position is used as the ray origin
	// instead of the position on the camera plane and the position must always
	// be in bounds

	texture_t res = TEX_EMPTY;
	// this remains unchanged if the ray doesnt hit anything

	while (map_is_in_bounds(map, cell_pos_x, cell_pos_y)) {
		RENDER_DEBUG(printf("now with s=(%f %f %f), cell pos = (%d %d) and last_stepped_x = %b\n",
			s.x, s.y, s.z, cell_pos_x, cell_pos_y, last_stepped_x));
		cell_t hit_cell = map_get_cell(map, cell_pos_x, cell_pos_y);
		if (hit_cell == CELL_UNTEXTURED_WALL || 0 <= hit_cell && hit_cell < 10) {
			RENDER_DEBUG(printf("hit untextured wall at (%d %d)\n", cell_pos_x, cell_pos_y));
			res =  last_stepped_x ? TEX_UNTEXTURED_WALL_Y : TEX_UNTEXTURED_WALL_X;
			break;
		}
		if (0 <= hit_cell && hit_cell < 10) {
			// textured wall
			RENDER_DEBUG(printf("hit textured wall (%d) in cell (%d %d)\n", hit_cell, cell_pos_x, cell_pos_y));
			res = hit_cell;
			break;
		}
		ASSERT(hit_cell == CELL_EMPTY, __LINE__, __FILE__" (invalid cell value)");

		// now step
		if (s.x < s.y) {
			s.x += t.x;
			cell_pos_x += r_x_sign;
			last_stepped_x = true;
		} else {
			s.y += t.y;
			cell_pos_y += r_y_sign;
			last_stepped_x = false;
		}
	}

	if (out_intersection) {
		if (last_stepped_x) {
			*out_intersection = vec3_add(i, vec3_mul_scalar(s.x - t.x, r));
		} else {
			*out_intersection = vec3_add(i, vec3_mul_scalar(s.y - t.y, r));
		}
		// TODO explain why you have to subtract t here
		RENDER_DEBUG(printf("wall hit at (%f %f %f)\n", out_intersection->x, out_intersection->y, out_intersection->z));
	}

	return res;
}


void render(FILE* out, render_buf_t *buf, map_t *map, int w, int h, double px, 
		double py, double fov, double rotation) {
	ASSERT(w % 2 == 0, __LINE__, __FILE__" (cant render odd widths)");
	ASSERT(h % 2 == 0, __LINE__, __FILE__" (cant render odd heights)");
	
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

		vec3_t wall_point;
		texture_t hit_tex = cast_ray(map, p, r, &wall_point);
		// DEBUG cast ray from p instead of i, since it goes through p anyways
		// and casting from i breaks bc it doesnt get in bounds

		double wall_distance = vec3_magnitude(vec3_sub(wall_point, vec3_sub(p, CAMERA_OFFSET)));
		// DEBUG when casting from i remember to replace this               ^ p with i too
		RENDER_DEBUG(printf("wall distance: %f\n", wall_distance));

		if (hit_tex == TEX_EMPTY) {
			// the entire column doesnt hit
			// so fill the upper part with ceiling and the lower with floor
			// note that h is even
			for (int y = 0; y < h/2; y++) {
				buf->pixels[x + y * w] = TEX_CEIL;
			}
			for (int y = h/2; y < h; y++) {
				buf->pixels[x + y * w] = TEX_FLOOR;
			}
			continue;
		}

		// in this case the horizontal ray does hit
		// TODO binary search the top/bottom of the wall or directly compute it?

		// we treat the wall as a plane with normal wall_normal = r and point wall_point
		//
		// double max_z = vec3_normalised(vec3_sub(vec3_add(wall_point, (vec3_t) {0, 0, 0.5}), p)).z;
		// double min_z = vec3_normalised(vec3_sub(vec3_add(wall_point, (vec3_t) {0, 0, -0.5}), p)).z;
		// // printf("got max_z = %f and min_z = %f\n", max_z, min_z);
		for (int y = 0; y < h; y++) {
			i = project_pixel_onto_image_plane(w, h, x, y, c, d);
			r = vec3_normalised(vec3_sub(p, i));

			double intersection_z = p.z + r.z * wall_distance;

			if (intersection_z < 0.0001) {
				buf->pixels[x + y * w] = TEX_FLOOR;
			} else if (intersection_z > 1.0001) {
				buf->pixels[x + y * w] = TEX_FLOOR;
			} else {
				buf->pixels[x + y * w] = hit_tex;
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
			case TEX_UNTEXTURED_WALL_X:
				ppm_write_colour(f, COLOUR_GREEN);
				break;
			case TEX_UNTEXTURED_WALL_Y:
				ppm_write_colour(f, COLOUR_RED);
				break;
			case TEX_CEIL:
			case TEX_FLOOR:
			case TEX_EMPTY:
				ppm_write_colour(f, COLOUR_BLACK);
				break;
			default:
				UNREACHABLE(__LINE__, __FILE__, "unexpected tex in draw_untextured");
		}
	}
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

