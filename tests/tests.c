#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <voxrnd/camera.h>
#include <voxrnd/simple-camera.h>
#include <voxrnd/distorted-camera.h>
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

static float dot_product (const vox_dot v1, const vox_dot v2)
{
    float res = 0;
    int i;

    for (i=0; i<VOX_N; i++) res += v1[i]*v2[i];
    return res;
}

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
    vox_quat basex = {cos(0.1), sinf(0.1), 0.0, 0.0};
    vox_dot vectx = {1.0, 0.0, 0.0};
    vox_dot resx;

    vox_quat basey = {cos(0.1), 0.0, sin(0.1), 0.0};
    vox_dot vecty = {0.0, 1.0, 0.0};
    vox_dot resy;

    vox_quat basez = {cos(0.1), 0.0, 0.0, sin(0.1)};
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
    
    vox_quat basex = {sincos, 0.0, sincos, 0.0};
    vox_dot vectx = {1.0, 0.0, 0.0};
    vox_dot expx = {0.0, 0.0, -1.0};

    vox_quat basey = {sincos, 0.0, 0.0, sincos};
    vox_dot vecty = {0.0, 1.0, 0.0};
    vox_dot expy = {-1.0, 0.0, 0.0};

    vox_quat basez = {sincos, sincos, 0.0, 0.0};
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
    
    vox_quat basex = {sincos, sincos, 0.0, 0.0};
    vox_quat basey = {sincos, 0.0, sincos, 0.0};
    
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
    vox_quat base1 = {cosf(phi), sinf(phi), 0, 0};
    // Then round y by 0.4 radian
    vox_quat base2 = {cosf(psi), 0, sinf(psi), 0};
    // Result rotation is
    vox_quat base_res = {cosf(phi)*cosf(psi), sinf(phi)*cosf(psi), sinf(psi)*cosf(phi), sinf(psi)*sinf(phi)};

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

    CU_ASSERT (fabsf (dot_product (vect, vect) - dot_product (res, res)) < PREC);
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
    vox_quat q11 = {0, 1, 0, 0};
    vox_quat q12 = {0, 0, 1, 0};
    vox_quat e1  = {0, 0, 0, 1};
    vox_quat_mul (q11, q12, res);
    CU_ASSERT (quat_eq (e1, res));

    vox_quat q21 = {0, 0, 1, 0};
    vox_quat q22 = {0, 0, 0, 1};
    vox_quat e2  = {0, 1, 0, 0};
    vox_quat_mul (q21, q22, res);
    CU_ASSERT (quat_eq (e2, res));

    vox_quat q31 = {0, 0, 0, 1};
    vox_quat q32 = {0, 1, 0, 0};
    vox_quat e3  = {0, 0, 1, 0};
    vox_quat_mul (q31, q32, res);
    CU_ASSERT (quat_eq (e3, res));

    // And in another direction...
    vox_quat q41 = {0, 0, 1, 0};
    vox_quat q42 = {0, 1, 0, 0};
    vox_quat e4  = {0, 0, 0, -1};
    vox_quat_mul (q41, q42, res);
    CU_ASSERT (quat_eq (e4, res));

    vox_quat q51 = {0, 0, 0, 1};
    vox_quat q52 = {0, 0, 1, 0};
    vox_quat e5  = {0, -1, 0, 0};
    vox_quat_mul (q51, q52, res);
    CU_ASSERT (quat_eq (e5, res));

    vox_quat q61 = {0, 1, 0, 0};
    vox_quat q62 = {0, 0, 0, 1};
    vox_quat e6  = {0, 0, -1, 0};
    vox_quat_mul (q61, q62, res);
    CU_ASSERT (quat_eq (e6, res));

    // Neutral element
    vox_quat qn1 = {4, 3, 2, 1};
    vox_quat n =   {1, 0, 0, 0};
    vox_quat_mul (qn1, n, res);
    CU_ASSERT (quat_eq (res, qn1));
    vox_quat_mul (n, qn1, res);
    CU_ASSERT (quat_eq (res, qn1));
    
    // Some random quaterions
    vox_quat q1 = {4, 1, 2, 3};
    vox_quat q2 = {2, 0, 2, 0};
    vox_quat e  = {4, -4, 12, 8};
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

// Various deletion cases
static void test_tree_del1()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i;
    for (i=0; i<VOX_MAX_DOTS+10; i++)
    {
        vox_insert_voxel (&tree, dot);
        dot[1]++;
    }
    CU_ASSERT (tree->flags & DENSE_LEAF);

    dot[1] = 0;
    for (i=0; i<10; i++)
    {
        vox_delete_voxel (&tree, dot);
        dot[1]++;
        CU_ASSERT (tree->flags & DENSE_LEAF);
        check_tree (tree);
    }
    vox_destroy_tree (tree);
}

static void test_tree_del2()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i;
    for (i=0; i<VOX_MAX_DOTS+10; i++)
    {
        vox_insert_voxel (&tree, dot);
        dot[2]++;
    }
    CU_ASSERT (tree->flags & DENSE_LEAF);

    dot[2] = VOX_MAX_DOTS+9;
    for (i=0; i<10; i++)
    {
        vox_delete_voxel (&tree, dot);
        dot[2]--;
        CU_ASSERT (tree->flags & DENSE_LEAF);
        check_tree (tree);
    }
    vox_destroy_tree (tree);
}

static void test_tree_del3()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i;
    for (i=0; i<VOX_MAX_DOTS+10; i++)
    {
        vox_insert_voxel (&tree, dot);
        dot[0]++;
    }
    CU_ASSERT (tree->flags & DENSE_LEAF);

    dot[0] = 1;
    for (i=0; i<10; i++)
    {
        vox_delete_voxel (&tree, dot);
        dot[0]++;
        CU_ASSERT (!(tree->flags & LEAF_MASK));
        check_tree (tree);
    }
    vox_destroy_tree (tree);
}

static void test_tree_del4()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i,j,k;

    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            for (k=0; k<3; k++)
            {
                dot[0] = i;
                dot[1] = j;
                dot[2] = k;
                vox_insert_voxel (&tree, dot);
            }
        }
    }

    CU_ASSERT (tree->flags & DENSE_LEAF);
    dot[0] = 0; dot[1] = 0; dot[2] = 0;
    vox_delete_voxel (&tree, dot);
    CU_ASSERT (!(tree->flags & LEAF_MASK));
    check_tree (tree);
    vox_destroy_tree (tree);
}

static void test_tree_del5()
{
    struct vox_node *tree = NULL;
    vox_dot dot = {0,0,0};
    int i,j,k;

    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            for (k=0; k<3; k++)
            {
                dot[0] = i;
                dot[1] = j;
                dot[2] = k;
                vox_insert_voxel (&tree, dot);
            }
        }
    }

    CU_ASSERT (tree->flags & DENSE_LEAF);
    dot[0] = 1; dot[1] = 1; dot[2] = 1;
    vox_delete_voxel (&tree, dot);
    CU_ASSERT (!(tree->flags & LEAF_MASK));
    check_tree (tree);
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

    const struct vox_node *leaf = vox_ray_tree_intersection (tree, origin, dir, res);
    CU_ASSERT (leaf != NULL);
}

static void test_simp_camera ()
{
    vox_dot pos = {0,0,0}, newpos;
    vox_dot angles;
    struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (NULL);
    camera->iface->set_position (camera, pos);
    camera->iface->set_window_size (camera, 100, 100);

    camera->iface->get_position (camera, newpos);
    CU_ASSERT (vect_eq (pos, newpos));

    vox_dot world_coord;
    vox_dot world_coord_expected = {0, 0, 1};
    vox_dot world_coord_expected2 = {0, 1, 0};

    camera->iface->screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected2));

    angles[0] = M_PI/4; angles[1] = 0; angles[2] = 0;
    camera->iface->set_rot_angles (camera, angles);
    camera->iface->screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected));

    angles[0] = 0; angles[1] = M_PI/4; angles[2] = 0;
    camera->iface->rotate_camera (camera, angles);
    camera->iface->screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected)); // fixed point

    camera->iface->move_camera (camera, world_coord);
    camera->iface->get_position (camera, newpos);
    CU_ASSERT (vect_eq (world_coord, newpos));

    camera->iface->destroy_camera (camera);
}

static void test_camera_class_coercion ()
{
    vox_dot pos = {10,110,1110}, newpos;
    struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (NULL);
    camera->iface->set_position (camera, pos);
    camera->iface->set_window_size (camera, 100, 100);
    camera->iface->set_fov (camera, 16);

    struct vox_camera *camera2 = vox_distorted_camera_iface()->construct_camera (camera);
    camera2->iface->get_position (camera2, newpos);
    CU_ASSERT (vect_eq (pos, newpos));
    CU_ASSERT (camera2->iface->get_fov (camera2) == 16);
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

    test = CU_add_test (vox_suite, "Deletion (case 1)", test_tree_del1);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Deletion (case 2)", test_tree_del2);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Deletion (case 3)", test_tree_del3);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Deletion (case 4)", test_tree_del4);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (vox_suite, "Deletion (case 5)", test_tree_del5);
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

    test = CU_add_test (rnd_suite, "Simple camera", test_simp_camera);
    if (test == NULL) PROC_TEST_ERROR;

    test = CU_add_test (rnd_suite, "Camera class coercion", test_camera_class_coercion);
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
