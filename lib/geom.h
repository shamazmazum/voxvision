/**
   @file geom.h
   @brief Basic geometry functions
**/

#ifndef __GEOM_H_
#define __GEOM_H_

#include <stdint.h>
#include "params.h"

/**
   \brief Take sum of two vectors.
   
   \param res an array where the result is stored
   \return Third passed argument, whichs contains the sum
**/
float* sum_vector (const float*, const float*, float*);

/**
   \brief Calculate a subspace index for the dot.
   
   For N-dimentional space we have 2^N ways to place the dot
   around the center of subdivision. Find which way is the case.
   
   \param dot1 the center of subdivision
   \param dot2 the dot we must calculate index for
   \return The subspace index in the range [0,2^N-1]
**/
uint8_t get_subspace_idx (const float*, const float*);

/**
   \brief Calc metric between two dots.
   
   A formula used is \f$\rho(x,y) = \Sigma_{i=1}^N \vert x_i - y_i \vert\f$
**/
float calc_abs_metric (const float*, const float*);

/**
   \brief Calc metric between two dots (variant 2).
   
   A formula used is \f$\rho (x,y) = \Sigma_{i=1}^N (x_i-y_i)^2\f$
   A square of usual euclid metric.
**/
float calc_sqr_metric (const float*, const float*);

int fit_into_box (const float*, const float*, const float*, float*);
int dot_betweenp (const float*, const float*, const float*);

/**
   \brief Find intersection of a ray and an axis-aligned box.
   
   \param min minimal coordinates of the box
   \param max maximal coordinates of the box
   \param origin a starting point of the ray
   \param dir the direction
   \param res where intersection is stored
   \return 1 if intersection is found, 0 otherwise
**/
int hit_box (const float*, const float*, const float*, const float*, float*);

/**
   \brief Find intersection of a ray and a plane.
   
   Plane must be axis-aligned
   \param origin a starting point of the ray
   \param dir the direction
   \param planedot a dot on the plane
   \param planenum an axis number the plane is aligned with
   \param res where intersection is stored
   \return 1 if intersection is found, 0 otherwise
**/
int hit_plane (const float*, const float*, const float*, int, float*);

/**
   \brief Find intersection of a box and a ball.
   
   The box and the ball are solid
**/
int box_ball_interp (const float*, const float*, const float*, float);
float* closest_in_set (float[][VOX_N], int, const float*, float (*) (const float*, const float*));

#endif
