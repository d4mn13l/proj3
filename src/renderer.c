#include "renderer.h"

#include <math.h>
#include <stdbool.h>

#include "map.h"
#include "maths.h"
#include "ppm.h"
#include "util.h"



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

	RENDER_DEBUG(printf("casting ray from %f %f %f in dir (%f %f %f)\n",
	       i.x, i.y, i.z, r.x, r.y, r.z));

	// travel distance for a full unit in x/y direction
	// called s_w and s_h in the project description
	vec3_t t = {fabs(1/r.x), fabs(1/r.y), 0};
	// we actually do not need to care if r.x is 0, since the 1/r.x will be
	// inf, so s.x will also be inf, therefore never be smaller than s.y and
	//  never be the direction of the next step
	RENDER_DEBUG(printf("t: %f %f %f\n", t.x, t.y, t.z));

	vec3_t in_cell_pos = {i.x - (int) i.x, i.y - (int) i.y, 0};
	// the position inside of the cell, so only the decimal part of the
	// position we want something in the range [0, 1), however this
	// calculation  can result in negative values if the original position
	// is also negative, but always in the range (-1, 1)
	if (in_cell_pos.x < 0) in_cell_pos.x += 1;
	if (in_cell_pos.y < 0) in_cell_pos.y += 1;
	// so this brings the value in the desired range

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
	res.hit = CELL_EMPTY;
	// this remains unchanged if the ray doesnt hit anything
	res.cell_x = i.x < 0 ? (int) i.x - 1 : (int) i.x;
	res.cell_y = i.y < 0 ? (int) i.y - 1 : (int) i.y;
	res.ray_direction = r;

	while (map_is_in_bounds(map, res.cell_x, res.cell_y)) {
		RENDER_DEBUG(printf("now with s=(%f %f %f), cell pos = (%d %d)"\
				" and last_stepped_x = %b\n", s.x, s.y, s.z,
				res.cell_x, res.cell_y, last_stepped_x));
		cell_t hit_cell = map_get_cell(map, res.cell_x, res.cell_y);

		if (hit_cell != CELL_EMPTY) {
			res.hit = hit_cell;
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
	// flip this because uhh idk
	// theres probably another flip somewhere else that cancels this out
	if (last_stepped == DIR_X) {
		res.position = vec3_add(i, vec3_mul_scalar(s.x - t.x, r));
	} else {
		res.position = vec3_add(i, vec3_mul_scalar(s.y - t.y, r));
	}

	return res;
}


 void render(image_t *img, map_t *map, size_t w, size_t h, double px,
		double py, double fov, double rotation, draw_function_t draw,
		tex_atlas_t *tex_atlas, bool do_shading) {
	
	ASSERT(0 < fov && fov < 180, __LINE__, __FILE__);
	
	// convert fov and rotation to radians
	fov = deg_to_rad(fov);
	rotation = deg_to_rad(rotation);

	RENDER_DEBUG(printf("w: %d, h: %d, px: %f, py: %f fov: %f, rot: %f\n",
			w,h,px,py,fov,rotation));

	// player position and aparture
	vec3_t p = {px, py, 0.5};

	// direction vector
	vec3_t d = {cos(rotation), sin(rotation), 0};

	// focal distance
	double f = w / (2 * tan(fov/2));

	// principle point of the image
	vec3_t c = vec3_sub(p, vec3_mul_scalar(f, d));

	RENDER_DEBUG(printf("player pos: (%f %f %f), focal distance: %f, "\
			"direction: (%f %f %f), priciple point: (%f %f %f)\n",
			p.x, p.y, p.z, f, d.x, d.y, d.z, c.x, c.y, c.z));

	for (size_t x = 0; x < w; x++) {
		// first cast a ray parallel to the floor from z=0
		// this checks if there is even a wall on that column at all

		vec3_t i = project_pixel_onto_image_plane(w, h, x, 0, c, d);
		i.z = 0.5;
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

		double wall_distance = vec3_magnitude(
		 vec3_sub(rc_res.position, p));
		RENDER_DEBUG(printf("wall distance: %f\n", wall_distance));

		// now loop over all rows (y pixels) in this column
		vec3_t r = horizontal_r;
		// new direction for the z-angled rays
		for (size_t y = 0; y < h; y++) {
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

			i = project_pixel_onto_image_plane(w, h, x, y, c, d);
			r = vec3_normalised(vec3_sub(p, i));
			r = vec3_mul_scalar(horizontal_r.x / r.x, r);
			// this makes sure that r.x == horizontal_r.x and
			// r.y == horizontal_r.y (as mentioned above).
			// since r points in the same x and y direction as
			// original_r, original_r.x / r.x == original_r.y / r.y
			// so it doesnt matter which one we take here

			// we only need to compute the z coordinate of the
			// intersection, not x and y since they stay the same
			rc_res.position.z = p.z + r.z * wall_distance;

			// we can leave the other values is rc_res as they dont
			// change
			draw(img, tex_atlas, &rc_res, do_shading, x, y);
		}
	}
}


void draw_untextured(image_t *img, tex_atlas_t *ta, ray_cast_result_t *rc_res,
		bool do_shading, size_t px, size_t py) {
	if (rc_res->hit == CELL_EMPTY || rc_res->position.z < 0.0001 ||
			rc_res->position.z > 0.9999) {
		img->pixels[px + py * img->w] = COLOUR_BLACK;
		return;
	}
	switch (rc_res->wall_orientation) {
		case DIR_X:
			img->pixels[px + py * img->w] = COLOUR_GREEN;
			return;
		case DIR_Y:
			img->pixels[px + py * img->w] = COLOUR_RED;
	}
}

void draw_textured_no_floor(image_t *img, tex_atlas_t *ta,
		ray_cast_result_t *rc_res, bool do_shading,
		size_t px, size_t py) {
	if (rc_res->hit == CELL_EMPTY || rc_res->position.z < 0.0001 ||
			rc_res->position.z > 0.9999) {
		img->pixels[px + py * img->w] = COLOUR_BLACK;
		return;
	}
	size_t tx, ty;
	//texture coordinates
	ty = (ta->h - 1) - (size_t) (rc_res->position.z * (double) ta->h);

	switch (rc_res->wall_orientation) {
	case DIR_X:
		tx = (size_t) ((rc_res->position.x - (int) rc_res->position.x) * (double) ta->w);
		if (rc_res->ray_direction.y < 0) tx = (ta->w - 1) - tx;
		break;
	case DIR_Y:
		tx = (size_t) ((rc_res->position.y - (int) rc_res->position.y) * (double) ta->w);
		if (rc_res->ray_direction.x > 0) tx = (ta->w - 1) - tx;
		break;
	default:
		UNREACHABLE(__LINE__, __FILE__, "invalid wall_orientaion");
	}

	tx = (ta->w - 1) - tx;
	size_t tex_index = rc_res->hit;
	if (tex_index == CELL_UNTEXTURED_WALL) tex_index = 2;
	// as specified in the project document

	ASSERT(tex_index < ta->count, __LINE__, __FILE__);

	colour_t *pixel = &img->pixels[px + py * img->w];

	*pixel = ta->tex[tex_index]->pixels[tx + ty * ta->w];

	if (do_shading && rc_res->wall_orientation == DIR_X) {
		pixel->r = (pixel->r >> 1) & 0x7F;
		pixel->g = (pixel->g >> 1) & 0x7F;
		pixel->b = (pixel->b >> 1) & 0x7F;
	}
}
 

void tex_atlas_load(tex_atlas_t *tex_atlas, FILE *f, size_t nx, size_t ny) {
	image_t img;
	ppm_image_load(&img, f);

	tex_atlas->tex = ppm_image_split(&img, nx, ny);
	// this implicitly confirms that nx and ny are non-zero
	ppm_image_free(&img);

	tex_atlas->nx = nx;
	tex_atlas->ny = ny;
	tex_atlas->w = tex_atlas->tex[0]->w;
	tex_atlas->h = tex_atlas->tex[0]->h;
	tex_atlas->count = nx * ny;
}
