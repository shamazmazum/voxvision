#include "local-loop.h"
#include "../voxtrees/search.h"

int vox_local_loop (struct vox_node *tree, void (*action) (vox_ll_context*), \
                     void (*inc) (vox_ll_context*), vox_ll_context *ctx, int mode)
{
    int state, pn;
    int depth = 1;
    int i;
    int n = VOX_LL_GET_ITER (mode);
    vox_tree_path path;

    pn = vox_ray_tree_intersection (tree, ctx->origin, ctx->dir, ctx->inter, 1, path);
    state = pn;
    if (state) action (ctx);
    inc (ctx);

    for (i=1; i<n; i++)
    {
        if ((mode&VOX_LL_ADAPTIVE) && (!state)) return i;
        if (state)
        {
            depth = vox_local_rays_tree_intersection (path, ctx->origin, ctx->dir, ctx->inter, depth, pn);
            state = depth;
        }
        if (!(state)) depth = vox_ray_tree_intersection (tree, ctx->origin, ctx->dir, ctx->inter, 1, NULL);
        
        if (depth) action (ctx);
        inc (ctx);
    }
    return i;
}
