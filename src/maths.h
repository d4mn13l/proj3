#ifndef MATHS_H
#define MATHS_H

#include <stdbool.h>

#define PI 3.14159265358979323846

#define VEC3_SPLIT(v) v.x, v.y, v.z
// helper for prinf-ing vec3s

typedef struct {
	double x, y, z;
} vec3_t;


vec3_t vec3_add(vec3_t v1, vec3_t v2);
vec3_t vec3_sub(vec3_t v1, vec3_t v2);
vec3_t vec3_mul_scalar(double a, vec3_t v);
double vec3_dot_product(vec3_t v1, vec3_t v2);

vec3_t vec3_normalised(vec3_t v);
double vec3_magnitude(vec3_t v);

double deg_to_rad(double deg);

static inline double fmod1(double d) {
	double res = d - (int) d;
	if (res < 0) return res + 1;
	return res;
}

bool is_approx_zero(double a);

#endif
