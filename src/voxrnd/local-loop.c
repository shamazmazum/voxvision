#include "local-loop.h"
#include "../voxtrees/search.h"

void vox_local_loop (struct vox_node *tree, int n, void (*action) (vox_rnd_context*), \
                     void (*inc) (vox_rnd_context*), vox_rnd_context *ctx)
{
    int i, state, pn;
    int depth = 1;
    vox_tree_path path;

    pn = vox_ray_tree_intersection (tree, ctx->origin, ctx->dir, ctx->inter, 1, path);
    state = pn;
    if (state) action (ctx);
    inc (ctx);

    for (i=1; i<n; i++)
    {
        if (state)
        {
            depth = vox_local_rays_tree_intersection (path, ctx->origin, ctx->dir, ctx->inter, depth, pn);
            state = depth;
        }
        if (!(state)) depth = vox_ray_tree_intersection (tree, ctx->origin, ctx->dir, ctx->inter, 1, path);
        
        if (depth) action (ctx);
        inc (ctx);
    }
}
