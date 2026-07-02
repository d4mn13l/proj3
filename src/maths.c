#include "maths.h"

#include <math.h>


float deg_to_rad(float deg) {
	return deg * (PI / 180);
}


int sign(float f) {
	return -2 * (*(int *) & f >> 31) - 1;
	// this uses the fact that the first bit of a float (so at 2^31) is
	// used to store the sign
}


vec3_t vec3_add(vec3_t v1, vec3_t v2) {
	return (vec3_t) {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vec3_t vec3_sub(vec3_t v1, vec3_t v2) {
	return (vec3_t) {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

vec3_t vec3_mul_scalar(float a, vec3_t v) {
	return (vec3_t) {a * v.x, a * v.y, a * v.z};
}

float vec3_dot_product(vec3_t v1, vec3_t v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float vec3_length(vec3_t v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalised(vec3_t v) {
	return vec3_mul_scalar(1 / vec3_length(v), v);
}

vec3_t vec3_rotate_y(vec3_t v, float angle) {
	return (vec3_t) {
		cos(angle) * v.x - sin(angle) * v.y,
		sin(angle) * v.x + cos(angle) * v.y,
		v.z
	};
}

bool is_approx_zero(float a) {
	return fabs(a) < 0.0001;
}
