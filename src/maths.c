#include "maths.h"

#include <math.h>


double deg_to_rad(double deg) {
	return deg * (PI / 180);
}


vec3_t vec3_add(vec3_t v1, vec3_t v2) {
	return (vec3_t) {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vec3_t vec3_sub(vec3_t v1, vec3_t v2) {
	return (vec3_t) {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

vec3_t vec3_mul_scalar(double a, vec3_t v) {
	return (vec3_t) {a * v.x, a * v.y, a * v.z};
}

double vec3_dot_product(vec3_t v1, vec3_t v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

double vec3_magnitude(vec3_t v) {
	return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
}

vec3_t vec3_normalised(vec3_t v) {
	return vec3_mul_scalar(1/vec3_magnitude(v), v);
}
