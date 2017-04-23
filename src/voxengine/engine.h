/**
   @file engine.h
   @brief Lua engine
**/
#ifndef ENGINE_H
#define ENGINE_H

struct vox_engine;

struct vox_engine* vox_create_engine (int *argc, char **argv[]);
void vox_engine_tick (struct vox_engine *engine);
void vox_destroy_engine (struct vox_engine *engine);

#endif
