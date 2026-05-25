#ifndef MATHS_H
#define MATHS_H


#define PI 3.14159265358979323846


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

#endif
