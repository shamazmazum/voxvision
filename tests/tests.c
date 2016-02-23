#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <voxrnd.h>
#include <voxrnd/vect-ops.h>
#include <voxtrees.h>
#include <voxtrees/geom.h>

#define PROC_SUIT_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a suite\n");        \
        goto regcleanup;}                          \
    while (0)

#define PROC_TEST_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a test\n");         \
        goto regcleanup;}                          \
    while (0)

#define PREC 0.0001

static struct vox_node *working_tree;
static vox_dot half_voxel;
static vox_dot neg_half_voxel;

static int dot_betweenp (const struct vox_box *box, const vox_dot dot)
{
    int i;

    for (i=0; i<VOX_N; i++) {if ((dot[i] < box->min[i]) || (dot[i] > box->max[i])) return 0;}
    return 1;
}

static int vect_eq (vox_dot v1, vox_dot v2)
{
    int i;
    float mdiff = PREC;
    for (i=0; i<3; i++)
    {
        float diff = fabsf (v1[i] - v2[i]);
        mdiff = (diff > mdiff) ? diff : mdiff;
    }

    if (mdiff <= PREC) return 1;
    else return 0;
}

static int quat_eq (vox_quat v1, vox_quat v2)
{
    int i;
    float mdiff = PREC;
    for (i=0; i<4; i++)
    {
        float diff = fabsf (v1[i] - v2[i]);
        mdiff = (diff > mdiff) ? diff : mdiff;
    }

    if (mdiff <= PREC) return 1;
    else return 0;
}

static void test_rotation_around_itself ()
{
    // Rotate vector around itself on 0.2 radian
    vox_quat basex = {sinf(0.1), 0.0, 0.0, cos(0.1)};
    vox_dot vectx = {1.0, 0.0, 0.0};
    vox_dot resx;

    vox_quat basey = {0.0, sin(0.1), 0.0, cos(0.1)};
    vox_dot vecty = {0.0, 1.0, 0.0};
    vox_dot resy;

    vox_quat basez = {0.0, 0.0, sin(0.1), cos(0.1)};
    vox_dot vectz = {0.0, 0.0, 1.0};
    vox_dot resz;
    
    vox_rotate_vector (basex, vectx, resx);
    CU_ASSERT (vect_eq (resx, vectx));
    
    vox_rotate_vector (basey, vecty, resy);
    CU_ASSERT (vect_eq (resy, vecty));

    vox_rotate_vector (basez, vectz, resz);
    CU_ASSERT (vect_eq (resz, vectz));
}

static void test_change_axis ()
{
    // Rotating x aroung y and getting z and so on
    float sincos = sqrtf(2.0)/2.0;
    
    vox_quat basex = {0.0, sincos, 0.0, sincos};
    vox_dot vectx = {1.0, 0.0, 0.0};
    vox_dot expx = {0.0, 0.0, -1.0};

    vox_quat basey = {0.0, 0.0, sincos, sincos};
    vox_dot vecty = {0.0, 1.0, 0.0};
    vox_dot expy = {-1.0, 0.0, 0.0};

    vox_quat basez = {sincos, 0.0, 0.0, sincos};
    vox_dot vectz = {0.0, 0.0, 1.0};
    vox_dot expz = {0.0, -1.0, 0.0};
    
    vox_rotate_vector (basex, vectx, vectx);
    CU_ASSERT (vect_eq (expx, vectx));
    
    vox_rotate_vector (basey, vecty, vecty);
    CU_ASSERT (vect_eq (expy, vecty));

    vox_rotate_vector (basez, vectz, vectz);
    CU_ASSERT (vect_eq (expz, vectz));
}

static float* vector_inv (const vox_dot dot, vox_dot res)
{
    int i;
    for (i=0; i<VOX_N; i++) res[i] = -dot[i];
    return res;
}

static void test_anticommut ()
{
    // Rotating x around y must result in rotaiong y around x with different sign
    float sincos = sqrtf(2.0)/2.0;
    
    vox_quat basex = {sincos, 0.0, 0.0, sincos};
    vox_quat basey = {0.0, sincos, 0.0, sincos};
    
    vox_dot vectx = {1.0, 0.0, 0.0};
    vox_dot vecty = {0.0, 1.0, 0.0};
    
    vox_dot res1, res2;
    vox_rotate_vector (basex, vecty, res1);
    vox_rotate_vector (basey, vectx, res2);
    
    CU_ASSERT (vect_eq (res1, vector_inv (res2, res2)));
}

static void rot_composition ()
{
    // Provided rotations q1, q2 and vector v test if q1 `rot` (q2 `rot` v) == q3 `rot` v,
    // where q3 = q1*q2
    float phi = 0.1;
    float psi = 0.2;

    // First rotate around x by 0.2 radian
    vox_quat base1 = {sinf(phi), 0, 0, cosf(phi)};
    // Then round y by 0.4 radian
    vox_quat base2 = {0, sinf(psi), 0, cosf(psi)};
    // Result rotation is
    vox_quat base_res = {sinf(phi)*cosf(psi), sinf(psi)*cosf(phi), sinf(psi)*sinf(phi), cosf(phi)*cosf(psi)};

    // Random numbers
    vox_dot vect = {132, 2, 35};

    vox_dot res1, res2;
    vox_rotate_vector (base2, vect, res1);
    vox_rotate_vector (base1, res1, res1);
    vox_rotate_vector (base_res, vect, res2);

    CU_ASSERT (vect_eq (res1, res2));
}

static void rot_saves_norm ()
{
    vox_quat base = {0.5, 0.5, 0.5, 0.5};
    // Random numbers
    vox_dot vect = {1, 5, 2};
    vox_dot res;
    vox_rotate_vector (base, vect, res);

    CU_ASSERT (fabsf (vox_dot_product (vect, vect) - vox_dot_product (res, res)) < PREC);
}

static struct vox_node* prepare_rnd_set_and_tree ()
{
    int i,j,k;
    size_t counter = 0;
    vox_dot *set = malloc (sizeof (vox_dot) * 1000000);
    // Make a ball with radius 50 and center (0, 0, 0)
    for (i=-50; i<50; i++)
    {
        for (j=-50; j<50; j++)
        {
            for (k=-50; k<50; k++)
            {
                int dist = i*i + j*j + k*k;
                if (dist < 2500)
                {
                    set[counter][0] = i;
                    set[counter][1] = j;
                    set[counter][2] = k;
                    counter++;
                }
            }
        }
    }
    return  vox_make_tree (set, counter);
}

static void check_tree (struct vox_node *tree)
{
    vox_dot tmp;
    int i;

    if (VOX_FULLP (tree))
    {
        if (tree->flags & DENSE_LEAF)
        {
            /*
              Dense leafs contain an exact number of dots depending on
              volume of their bounding boxes
            */
            vox_dot size;
            for (i=0; i<VOX_N; i++)
                size[i] = tree->bounding_box.max[i] - tree->bounding_box.min[i];
            float bb_volume = size[0]*size[1]*size[2];
            bb_volume /= vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
            CU_ASSERT (vox_voxels_in_tree (tree) == (int)bb_volume);
        }
        else if (tree->flags & LEAF)
        {
            vox_dot *dots = tree->data.dots;
            // Check that all voxels are covered by bounding box
            for (i=0; i<tree->dots_num; i++)
            {
                sum_vector (dots[i], vox_voxel, tmp);
                CU_ASSERT (dot_betweenp (&(tree->bounding_box), dots[i]));
                CU_ASSERT (dot_betweenp (&(tree->bounding_box), tmp));
            }
        }
        else
        {
            vox_inner_data inner = tree->data.inner;
            size_t number_sum = 0;
            for (i=0; i<VOX_NS; i++)
            {
                struct vox_node *child = inner.children[i];
                number_sum += vox_voxels_in_tree (child);
                if (VOX_FULLP (child))
                {
                    // Check if child's bounding box is inside parent's
                    CU_ASSERT (dot_betweenp (&(tree->bounding_box), child->bounding_box.min));
                    CU_ASSERT (dot_betweenp (&(tree->bounding_box), child->bounding_box.max));

                    /*
                      Check subspace. We add/subtract a small value (half size of a voxel)
                      to bounding box corners because faces of bounding box may belong to
                      another subspace. What is inside may not.
                    */
                    sum_vector (child->bounding_box.min, half_voxel, tmp);
                    CU_ASSERT (get_subspace_idx (inner.center, tmp) == i);
                    sum_vector (child->bounding_box.max, neg_half_voxel, tmp);
                    CU_ASSERT (get_subspace_idx (inner.center, tmp) == i);
                }
                // Test a child recursively
                check_tree (child);
            }
            CU_ASSERT (vox_voxels_in_tree (tree) == number_sum);
        }
    }
}

static void quat_mul ()
{
    vox_quat res;
    
    // Check multiplication table
    vox_quat q11 = {1, 0, 0, 0};
    vox_quat q12 = {0, 1, 0, 0};
    vox_quat e1  = {0, 0, 1, 0};
    vox_quat_mul (q11, q12, res);
    CU_ASSERT (quat_eq (e1, res));

    vox_quat q21 = {0, 1, 0, 0};
    vox_quat q22 = {0, 0, 1, 0};
    vox_quat e2  = {1, 0, 0, 0};
    vox_quat_mul (q21, q22, res);
    CU_ASSERT (quat_eq (e2, res));

    vox_quat q31 = {0, 0, 1, 0};
    vox_quat q32 = {1, 0, 0, 0};
    vox_quat e3  = {0, 1, 0, 0};
    vox_quat_mul (q31, q32, res);
    CU_ASSERT (quat_eq (e3, res));

    // And in another direction...
    vox_quat q41 = {0, 1, 0, 0};
    vox_quat q42 = {1, 0, 0, 0};
    vox_quat e4  = {0, 0, -1, 0};
    vox_quat_mul (q41, q42, res);
    CU_ASSERT (quat_eq (e4, res));

    vox_quat q51 = {0, 0, 1, 0};
    vox_quat q52 = {0, 1, 0, 0};
    vox_quat e5  = {-1, 0, 0, 0};
    vox_quat_mul (q51, q52, res);
    CU_ASSERT (quat_eq (e5, res));

    vox_quat q61 = {1, 0, 0, 0};
    vox_quat q62 = {0, 0, 1, 0};
    vox_quat e6  = {0, -1, 0, 0};
    vox_quat_mul (q61, q62, res);
    CU_ASSERT (quat_eq (e6, res));

    // Neutral element
    vox_quat qn1 = {4, 3, 2, 1};
    vox_quat n =   {0, 0, 0, 1};
    vox_quat_mul (qn1, n, res);
    CU_ASSERT (quat_eq (res, qn1));
    vox_quat_mul (n, qn1, res);
    CU_ASSERT (quat_eq (res, qn1));
    
    // Some random quaterions
    vox_quat q1 = {1, 2, 3, 4};
    vox_quat q2 = {0, 2, 0, 2};
    vox_quat e  = {-4, 12, 8, 4};
    vox_quat_mul (q1, q2, res);
    CU_ASSERT (quat_eq (res, e));
}

static void test_tree_cons () {check_tree (working_tree);}

static void test_tree_ins()
{
    struct vox_node *tree = NULL;
    vox_dot dot1 = {0, 0, 0};
    vox_dot dot11 = {0.5, 0.1, 0.8};
    vox_dot dot2 = {6, 6, 6};
    int res;

    // Insert the first voxel
    res = vox_insert_voxel (&tree, dot1);
    CU_ASSERT (res);
    CU_ASSERT (VOX_FULLP (tree) && vox_voxels_in_tree (tree) == 1);

    // Insert the same voxel again
    res = vox_insert_voxel (&tree, dot11);
    CU_ASSERT (!res);
    CU_ASSERT (VOX_FULLP (tree) && vox_voxels_in_tree (tree) == 1);
    // Insert another voxel
    res = vox_insert_voxel (&tree, dot2);
    CU_ASSERT (res);
    CU_ASSERT (VOX_FULLP (tree) && vox_voxels_in_tree (tree) == 2);
    check_tree (tree);
    vox_destroy_tree (tree);

    // Make a fake tree with dense leaf as root
    tree = aligned_alloc(16, sizeof (struct vox_node));
    tree->bounding_box.min[0] = 5;
    tree->bounding_box.min[1] = 5;
    tree->bounding_box.min[2] = 5;
    tree->bounding_box.max[0] = 10;
    tree->bounding_box.max[1] = 10;
    tree->bounding_box.max[2] = 10;
    tree->dots_num = 125;
    tree->flags = DENSE_LEAF;
    res = vox_insert_voxel (&tree, dot2);
    CU_ASSERT (!res);
    res = vox_insert_voxel (&tree, dot1);
    CU_ASSERT (res && vox_voxels_in_tree (tree) == 126);
    check_tree (tree);
    vox_destroy_tree (tree);

    tree = NULL;
    int i, j, k;
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            for (k=0; k<3; k++)
            {
                dot1[0] = 2*i;
                dot1[1] = 2*j;
                dot1[2] = 2*k;
                res = vox_insert_voxel (&tree, dot1);
                CU_ASSERT (res);
            }
        }
    }
    CU_ASSERT (vox_voxels_in_tree (tree) == 27);
    check_tree (tree);
    vox_destroy_tree (tree);

    // Check dense node creation
    tree = NULL;
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            for (k=0; k<3; k++)
            {
                dot1[0] = i;
                dot1[1] = j;
                dot1[2] = k;
                res = vox_insert_voxel (&tree, dot1);
                CU_ASSERT (res);
            }
        }
    }
    CU_ASSERT (vox_voxels_in_tree (tree) == 27);
    check_tree (tree);
    vox_destroy_tree (tree);
}

static void test_tree_del()
{
    vox_dot *set = aligned_alloc (16, sizeof(vox_dot)*27);
    int i,j,k,counter = 0;
    struct vox_node *tree;

    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            for (k=0; k<3; k++)
            {
                set[counter][0] = i*vox_voxel[0];
                set[counter][1] = j*vox_voxel[1];
                set[counter][2] = k*vox_voxel[2];
                counter++;
            }
        }
    }
    tree = vox_make_tree (set, 27);
    free (set);

    vox_dot dot;
    int n;
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            for (k=0; k<3; k++)
            {
                dot[0] = i*vox_voxel[0];
                dot[1] = j*vox_voxel[1];
                dot[2] = k*vox_voxel[2];
                n = vox_voxels_in_tree (tree);
                vox_delete_voxel (&tree, dot);
                CU_ASSERT (n - vox_voxels_in_tree (tree) == 1);
                check_tree (tree);
            }
        }
    }
    // Not really needed
    vox_destroy_tree (tree);
}

static void test_tree_ins_trans()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i;

    for (i=0; i<VOX_MAX_DOTS+4; i++)
    {
        dot[0]++;
        vox_insert_voxel (&tree, dot);
        CU_ASSERT (tree->flags & DENSE_LEAF);
    }
    vox_destroy_tree (tree);
}

static void test_tree_del_trans()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i;

    for (i=0; i<VOX_MAX_DOTS+4; i++)
    {
        dot[0]++;
        vox_insert_voxel (&tree, dot);
    }

    dot[0] = 0;
    for (i=0; i<VOX_MAX_DOTS+4; i++)
    {
        CU_ASSERT (tree->flags & DENSE_LEAF);
        dot[0]++;
        vox_delete_voxel (&tree, dot);
    }
    CU_ASSERT (!(VOX_FULLP (tree)));
}

static void test_tree_C676d50c2 ()
{
    struct vox_node *tree = NULL;
    vox_dot dot;

    dot[0] = 0; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);

    dot[0] = 0; dot[1] = 0; dot[2] = -1;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = 0; dot[2] = -2;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = 0; dot[2] = 1;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = 0; dot[2] = 2;
    vox_insert_voxel (&tree, dot);

    dot[0] = -1; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = -2; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = -3; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = -4; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);

    dot[0] = 1; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 2; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 3; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 4; dot[1] = 0; dot[2] = 0;
    vox_insert_voxel (&tree, dot);

    dot[0] = 0; dot[1] = -1; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = -2; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = -3; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = -4; dot[2] = 0;
    vox_insert_voxel (&tree, dot);

    dot[0] = 0; dot[1] = 1; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = 2; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = 3; dot[2] = 0;
    vox_insert_voxel (&tree, dot);
    dot[0] = 0; dot[1] = 4; dot[2] = 0;
    vox_insert_voxel (&tree, dot);

    vox_dot origin = {-2, -2, 0};
    vox_dot dir = {1, 1, -1};
    vox_dot res;

    int interp = vox_ray_tree_intersection (tree, origin, dir, res, NULL);
    CU_ASSERT (interp);
}

static struct vox_rnd_ctx* make_fake_context (vox_camera_interface *iface)
{
    SDL_Surface *surf = malloc (sizeof(SDL_Surface));
    surf->w = 100;
    surf->h = 100;
    struct vox_rnd_ctx* res = vox_make_renderer_context (surf, NULL, iface);
    return res;
}

static void free_fake_context (struct vox_rnd_ctx *ctx)
{
    free (ctx->surface);
    free (ctx);
}

static void test_simp_camera ()
{
    vox_dot pos = {0,0,0};
    vox_dot angles;
    vox_simple_camera *camera = vox_make_simple_camera (1.2, pos);
    struct vox_rnd_ctx *ctx = make_fake_context (&(camera->iface));

    CU_ASSERT (vect_eq (pos, camera->iface.get_position (camera)));

    vox_dot world_coord;
    vox_dot world_coord_expected = {0, 0, 1};

    angles[0] = M_PI/4; angles[1] = 0; angles[2] = 0;
    camera->iface.set_rot_angles (camera, angles);
    camera->iface.screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected));

    angles[0] = 0; angles[1] = M_PI/4; angles[2] = 0;
    camera->iface.rotate_camera (camera, angles);
    camera->iface.screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected)); // fixed point

    camera->iface.move_camera (camera, world_coord);
    CU_ASSERT (vect_eq (world_coord, camera->iface.get_position (camera)));

    free_fake_context (ctx);
    free (camera);
}

int main ()
{
    CU_pTest test;
    
    CU_ErrorCode res = CU_initialize_registry();
    if (res != CUE_SUCCESS)
    {
        printf ("Failed to initialize registry\n");
        goto exit_;
    }

    // Working with vectors
    CU_pSuite vect_suite = CU_add_suite ("vect-ops", NULL, NULL);
    if (vect_suite == NULL) PROC_SUIT_ERROR;

    test = CU_add_test (vect_suite, "rotate around itself", test_rotation_around_itself);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vect_suite, "change axis", test_change_axis);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vect_suite, "anticommutativity of some rotations", test_anticommut);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vect_suite, "composition of rotations", rot_composition);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vect_suite, "Rotation saves norm", rot_saves_norm);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vect_suite, "Quaternion multiplication", quat_mul);
    if (test == NULL) PROC_TEST_ERROR;

    // Tree construction and searching
    CU_pSuite vox_suite = CU_add_suite ("voxtrees", NULL, NULL);
    if (vox_suite == NULL) PROC_SUIT_ERROR;

    test = CU_add_test (vox_suite, "Tree construction", test_tree_cons);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Insertion", test_tree_ins);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Deletion", test_tree_del);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Insertion type transitions", test_tree_ins_trans);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Deletion type transitions", test_tree_del_trans);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Search (commit 676d50c2)", test_tree_C676d50c2);
    if (test == NULL) PROC_TEST_ERROR;

    // Renderer library
    CU_pSuite rnd_suite = CU_add_suite ("voxrnd", NULL, NULL);
    if (vox_suite == NULL) PROC_SUIT_ERROR;

    test = CU_add_test (rnd_suite, "Camera class", test_simp_camera);
    if (test == NULL) PROC_TEST_ERROR;
    

    printf ("Creating working set and working tree\n");
    working_tree = prepare_rnd_set_and_tree ();
    int i;
    for (i=0; i<VOX_N; i++)
    {
        half_voxel[i] = vox_voxel[i]/2;
        neg_half_voxel[i] = -vox_voxel[i]/2;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
regcleanup: CU_cleanup_registry();
exit_:    return res;
}
