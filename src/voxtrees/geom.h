/**
   @file geom.h
   @brief Basic geometry functions
**/

#ifndef __GEOM_H_
#define __GEOM_H_

#include "params.h"

/**
   \brief Take sum of two vectors.

   \param a
   \param b
   \param res an array where the result is stored
**/
void sum_vector (const vox_dot, const vox_dot, vox_dot);

/**
   \brief Calculate a subspace index for the dot.
   
   For N-dimentional space we have 2^N ways to place the dot
   around the center of subdivision. Find which way is the case.
   
   \param dot1 the center of subdivision
   \param dot2 the dot we must calculate index for
   \return The subspace index in the range [0,2^N-1]
**/
vox_uint get_subspace_idx (const vox_dot, const vox_dot);

/**
   \brief Calc metric between two dots.
   
   A formula used is \f$\rho(x,y) = \Sigma_{i=1}^N \vert x_i - y_i \vert\f$
**/
float calc_abs_metric (const vox_dot, const vox_dot);

/**
   \brief Calc metric between two dots (variant 2).
   
   A formula used is \f$\rho (x,y) = \Sigma_{i=1}^N (x_i-y_i)^2\f$
   A square of usual euclid metric.
**/
float calc_sqr_metric (const vox_dot, const vox_dot);

int fit_into_box (const vox_dot, const vox_dot, const vox_dot, vox_dot);
int dot_betweenp (const vox_dot, const vox_dot, const vox_dot);

/**
   \brief Find intersection of a ray and an axis-aligned box.
   
   \param min minimal coordinates of the box
   \param max maximal coordinates of the box
   \param origin a starting point of the ray
   \param dir the direction
   \param res where intersection is stored
   \return 1 if intersection is found, 0 otherwise
**/
int hit_box (const vox_dot, const vox_dot, const vox_dot, const vox_dot, vox_dot);

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
int hit_plane (const vox_dot, const vox_dot, const vox_dot, int, vox_dot);

/**
   \brief Find intersection of a box and a ball.
   
   The box and the ball are solid
**/
int box_ball_interp (const vox_dot, const vox_dot, const vox_dot, float);
float* closest_in_set (vox_dot[], int, const vox_dot, float (*) (const vox_dot, const vox_dot));

#endif