/**
   @file geom.h
   @brief Basic geometry functions
**/

#ifndef __GEOM_H_
#define __GEOM_H_

#include "params.h"

/**
   \brief Calculate fast metric between two dots.

   This is a fast metric on R^3 calculacted as sum of absolute values of
   by-coordinate differences between two dots.
**/
VOX_EXPORT float vox_abs_metric (const vox_dot dot1, const vox_dot dot2);

/**
   \brief Calculate metric between two dots.
   
   This is a square of standard Euclidian metric on R^3
**/
VOX_EXPORT float vox_sqr_metric (const vox_dot dot1, const vox_dot dot2);

/**
   \brief Calculate norm of a vector.

   This is a square of standard Euclidian norm on R^3
**/
VOX_EXPORT float vox_sqr_norm (const vox_dot dot);

#ifdef VOXTREES_SOURCE

// Stuff marked VOX_EXPORT is needed for tests.
// TODO: check it has vox_ prefix.

/**
   \brief Calculate a subspace index for the dot.
   
   For N-dimentional space we have 2^N ways to place the dot
   around the center of subdivision. Find which way is the case.
   
   \param dot1 the center of subdivision
   \param dot2 the dot we must calculate index for
   \return The subspace index in the range [0,2^N-1]
**/
VOX_EXPORT unsigned int get_subspace_idx (const vox_dot center, const vox_dot dot);

/*
 * Generally this works like get_subspace_idx () but corrects subspace
 * index if center[i] == dot[i]. In this case, direction[i] is used to
 * choose the index at i-th coordinate.
 */
unsigned int get_corrected_subspace_idx (const vox_dot center,
                                         const vox_dot dot,
                                         const vox_dot direction);


/**
   \brief Find intersection of a ray and an axis-aligned box.
   
   \param box a box to be checked
   \param origin a starting point of the ray
   \param dir the direction
   \param res where intersection is stored
   \return 1 if intersection is found, 0 otherwise
**/
VOX_EXPORT int vox_hit_box (const struct vox_box *box,
                            const vox_dot origin,
                            const vox_dot dir,
                            vox_dot *res);

/**
   \brief Find intersection of a ray and a plane.

   An intersection must be within a box.
   Plane must be axis-aligned.
   
   \param origin a starting point of the ray
   \param dir the direction
   \param planedot a dot on the plane
   \param planenum an axis number the plane is aligned with
   \param res where intersection is stored
   \param box a box
   \return 1 if intersection is found, 0 otherwise
**/
int hit_plane_within_box (const vox_dot origin, const vox_dot dir, const vox_dot planedot,
                          int planenum, vox_dot *res, const struct vox_box *box);

/**
   \brief Find intersection of a box and a ball.
   
   The box and the ball are solid
**/
VOX_EXPORT int vox_box_ball_interp (const struct vox_box *box,
                                    const vox_dot center,
                                    float radius);

int dense_set_p (const struct vox_box *box, size_t n);
VOX_EXPORT int vox_voxel_in_box (const struct vox_box *box, const vox_dot dot);
#endif

#endif
