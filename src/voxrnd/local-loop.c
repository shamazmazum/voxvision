#include "local-loop.h"
#include "../voxtrees/search.h"

void vox_local_loop (struct vox_node *tree, void (*action) (vox_rnd_context*), \
                     int (*iter_post) (vox_rnd_context*, int), vox_rnd_context *ctx)
{
    int state, pn;
    int depth = 1;
    int i = 0;
    int run;
    vox_tree_path path;

    pn = vox_ray_tree_intersection (tree, ctx->origin, ctx->dir, ctx->inter, 1, path);
    state = pn;
    if (state) action (ctx);
    run = iter_post (ctx, i);

    while (run)
    {
        if (state)
        {
            depth = vox_local_rays_tree_intersection (path, ctx->origin, ctx->dir, ctx->inter, depth, pn);
            state = depth;
        }
        if (!(state)) depth = vox_ray_tree_intersection (tree, ctx->origin, ctx->dir, ctx->inter, 1, path);
        
        if (depth) action (ctx);
        i++;
        run = iter_post (ctx, i);
    }
}
