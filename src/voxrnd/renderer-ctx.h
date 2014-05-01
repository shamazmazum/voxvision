/**
   @file renderer-ctx.h
   @brief Managing renderer context
**/
#ifndef RENDERER_CTX_H
#define RENDERER_CTX_H

#include <SDL/SDL.h>
#include "camera.h"
#include "../params_var.h"

#ifdef VOXRND_SOURCE
/**
   \brief Renderer context shared by renderer loop and user
**/
typedef struct
{
    vox_dot origin;  /**< \brief Starting point of intersecting ray */
    vox_dot dir;     /**< \brief Direction of intersecting ray */
    vox_dot inter;   /**< \brief Where intersections are stored */
    void *user_data; /**< \brief Data not used by renderer loop */
} vox_rnd_context;

/**
   \brief User data stored in renderer context
**/
typedef struct
{
    SDL_Surface *surface; /**< \brief SDL surface for output */
    vox_camera *camera;      /**< \brief camera object */
    int p;
    int sx, sy;

    // FIXME: this must not be here
    float col_mul[3];
    float col_add[3];
} vox_rnd_aux_ctx;
#else // VOXRND_SOURCE
typedef struct _vox_rnd_context_ vox_rnd_context;
#endif

/**
   \brief Create renderer context

   \param surf a SDL surface for output
   \param camera a camera
   \return the context
**/
vox_rnd_context* vox_init_renderer_context (SDL_Surface*, void*);

/**
   \brief Free renderer context
**/
void vox_free_renderer_context (vox_rnd_context*);

#endif
