#ifndef __GEOM_H_
#define __GEOM_H_

#include "params.h"

float calc_abs_metric (const float*, const float*);
float calc_sqr_metric (const float*, const float*);

int fit_into_box (const float*, const float*, const float*, float*);
int dot_betweenp (const float*, const float*, const float*);
int hit_box (const float*, const float*, const float*, const float*, float*);
int hit_plane (const float*, const float*, const float*, int, float*);
int box_ball_interp (const float*, const float*, const float*, float);
float* closest_in_set (float[][N], int, const float*, float (*) (const float*, const float*));

#endif
