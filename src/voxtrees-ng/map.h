#ifndef __MAP_H__
#define __MAP_H__

struct vox_map_3d {
    int *map;
    unsigned int dim[3];
    unsigned int total_size;
};

struct vox_map_3d* vox_create_map_3d (const unsigned int dim[3]);
void vox_destroy_map_3d (struct vox_map_3d *map);

#endif
