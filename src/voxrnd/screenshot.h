/**
   @file screenshot.h
   @brief Simple BMP screenshot function
**/
#ifndef SCREENSHOT_H
#define SCREENSHOT_H
#include "renderer.h"

/**
   \brief Save a screenshot.

   This function saves screenshot in BMP format. It does not create
   screenshot directory by itself.
   
   In the case of error, `SDL_GetError()` can be used to get the error.

   \param context A renderer context used to make screenshot.
   \param dirname Screenshot directory name. May be NULL which means
          local voxvision data directory.
   \return 1 on success, 0 on error.
**/
int vox_screenshot (const struct vox_rnd_ctx *context, const char *dirname);

#endif
