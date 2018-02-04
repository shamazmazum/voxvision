#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "tree.h"

const struct vox_node*
vox_ray_tree_intersection (const struct vox_node* tree, const vox_dot origin,
                           const vox_dot dir, vox_dot res);
int vox_tree_ball_collidep (const struct vox_node* tree, const vox_dot center, float radius);
#endif
