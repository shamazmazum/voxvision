#include "renderer.h"
#include "local-loop.h"

#define min(a,b) ((a)<(b)) ? (a) : (b)

struct renderer_ctx
{
    SDL_Surface *surface;
    vox_camera *camera;
    int p;
    int sx, sy;

    // FIXME: this must not be here
    float col_mul[3];
    float col_add[3];
};
static struct renderer_ctx rctx;

static void color_coeff (struct vox_node *tree, float mul[], float add[])
{
    int i;
    vox_dot min;
    vox_dot max;
    vox_bounding_box (tree, min, max);
    
    for (i=0; i<3; i++)
    {
        mul[i] = 255 / (max[i] - min[i]);
        add[i] = -255 * min[i] / (max[i] - min[i]);
    }
}

// Use vox_local_loop here. Render 1x4 line by iteration
// Increments sx and p, changes dir on each step
static void line_inc (vox_ll_context *ctx)
{
    rctx.p++;
    rctx.sx++;
    if (rctx.sx == rctx.surface->w) rctx.sx = 0;

    rctx.camera->screen2world (rctx.camera, ctx->dir, rctx.surface->w,
                               rctx.surface->h, rctx.sx, rctx.sy);
}

static Uint32 get_color (SDL_PixelFormat *format, vox_dot inter, float mul[], float add[])
{
    Uint8 r = mul[0]*inter[0]+add[0];
    Uint8 g = mul[1]*inter[1]+add[1];
    Uint8 b = mul[2]*inter[2]+add[2];
    Uint32 color = SDL_MapRGB (format, r, g, b);
    return color;
}

static void line_action (vox_ll_context *ctx)
{
    Uint32 color = get_color (rctx.surface->format, ctx->inter, rctx.col_mul, rctx.col_add);
    *((Uint32*)rctx.surface->pixels + rctx.p) = color;
    
}

void vox_render (struct vox_node *tree, vox_camera *camera, SDL_Surface *surface)
{
    // Init context before we start
    int w = surface->w;
    int h = surface->h;
    rctx.surface = surface;
    rctx.camera = camera;
    rctx.p = 0;
    rctx.sx = 0;
    rctx.sy = 0;

    // FIXME: calculate colors in runtime
    // Only a temporary solution to get a colorful output
    color_coeff (tree, rctx.col_mul, rctx.col_add);

    vox_ll_context llctx;
    vox_dot_copy (llctx.origin, camera->position);
    camera->screen2world (camera, llctx.dir, w, h, 0, 0);
    
    for (rctx.sy=0; rctx.sy<h; rctx.sy++)
    {
        do
        {
#if 1
            vox_local_loop (tree, line_action, line_inc, &llctx, VOX_LL_ADAPTIVE|VOX_LL_MAXITER(min (4, w-rctx.sx)));
#else
            vox_local_loop (tree, line_action, line_inc, &llctx, VOX_LL_FIXED|VOX_LL_MAXITER(4));
#endif
        }
        while (rctx.sx != 0);
    }
    SDL_Flip (surface);
}
