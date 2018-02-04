#include <stdlib.h>
#include <string.h>
#include "map.h"

struct vox_map_3d* vox_create_map_3d (const unsigned int dim[3])
{
    unsigned int total_size = dim[0] * dim[1] * dim[2];
    struct vox_map_3d *map = malloc (sizeof (struct vox_map_3d));
    memcpy (map->dim, dim, 3 * sizeof(int));
    map->total_size = total_size;
    size_t map_size = sizeof (int) * total_size;
    map->map = malloc (map_size);
    memset (map->map, 0, map_size);

    return map;
}

void vox_destroy_map_3d (struct vox_map_3d *map)
{
    if (map != NULL) {
        free (map->map);
        free (map);
    }
}
