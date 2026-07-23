#include "maths.h"

#include <math.h>


float deg_to_rad(float deg) {
	return deg * (PI / 180);
}


int signf(float f) {
	return -2 * ((*(int *) &f) >> 31) - 1;
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

float vec3_length_squared(vec3_t v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vec3_length(vec3_t v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalised(vec3_t v) {
	return vec3_mul_scalar(1 / vec3_length(v), v);
}

vec3_t vec3_rotate_y(vec3_t v, float angle) {
	return (vec3_t) {
		cosf(angle) * v.x - sinf(angle) * v.y,
		sinf(angle) * v.x + cosf(angle) * v.y,
		v.z
	};
}

float vec3_angle(vec3_t v, vec3_t u) {
	return acosf(vec3_dot_product(v, u) / (vec3_length(v) * vec3_length(u)));
}

float vec2_angle(vec3_t v, vec3_t u) {
	return acosf(vec2_dot_product(v, u) / (vec2_length(v) * vec2_length(u)));
}


float vec2_dot_product(vec3_t v1, vec3_t v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

float vec2_length_squared(vec3_t v) {
	return v.x * v.x + v.y * v.y;
}

float vec2_length(vec3_t v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

vec3_t vec2_normalised(vec3_t v) {
	return vec3_mul_scalar(1 / vec2_length(v), v);
}



bool vec3_equal_approx(vec3_t v1, vec3_t v2) {
	return is_approx_zero(v1.x - v2.x) && is_approx_zero(v1.y - v2.y)
		&& is_approx_zero(v1.z - v2.z);
}

bool is_approx_zero(float a) {
	return fabs(a) < 0.0001;
}
