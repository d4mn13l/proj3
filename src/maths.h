#ifndef MATHS_H
#define MATHS_H

#include <stdbool.h>

#define PI 3.14159265358979323846

#define VEC3_SPLIT(v) v.x, v.y, v.z
// helper for prinf-ing vec3s

typedef struct {
	float x, y, z;
} vec3_t;

typedef struct {
	float x, y;
} vec2_t;

vec3_t vec3_add(vec3_t v1, vec3_t v2);
vec3_t vec3_sub(vec3_t v1, vec3_t v2);
vec3_t vec3_mul_scalar(float a, vec3_t v);
float vec3_dot_product(vec3_t v1, vec3_t v2);

vec3_t vec3_normalised(vec3_t v);
float vec3_length(vec3_t v);


vec3_t vec3_rotate_y(vec3_t v, float angle);


float deg_to_rad(float deg);

static inline float fmod1(float d) {
	float res = d - (int) d;
	if (res < 0) return res + 1;
	return res;
}

bool is_approx_zero(float a);

#endif
