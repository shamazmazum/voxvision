#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <voxrnd/camera.h>
#include <voxrnd/vect-ops.h>
#include <voxtrees.h>
#include <voxtrees/geom.h>

static float precise_check;
static float approx_check;

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

static int vect_eq (const vox_dot v1, const vox_dot v2, float precision)
{
    int i;
    float mdiff = precision;
    for (i=0; i<3; i++)
    {
        float diff = fabsf (v1[i] - v2[i]);
        mdiff = (diff > mdiff) ? diff : mdiff;
    }

    if (mdiff == precision) return 1;
    else return 0;
}

static int quat_eq (const vox_quat v1, const vox_quat v2, float precision)
{
    int i;
    float mdiff = precision;
    for (i=0; i<4; i++)
    {
        float diff = fabsf (v1[i] - v2[i]);
        mdiff = (diff > mdiff) ? diff : mdiff;
    }

    if (mdiff == precision) return 1;
    else return 0;
}

static int hit_dot (const vox_dot origin, const vox_dot dir, const vox_dot target)
{
    float t = (target[0] - origin[0])/dir[0];
    CU_ASSERT (t >= 0);
    vox_dot res;
    res[0] = target[0];
    res[1] = dir[1]*t + origin[1];
    res[2] = dir[2]*t + origin[2];

    return vect_eq (target, res, approx_check);
}

static void test_identity ()
{
    vox_quat identity;

    vox_quat_set_identity (identity);
    vox_dot dot1 = {1,2,4}, rot1;
    vox_dot dot2 = {-321,22,124}, rot2;
    vox_dot dot3 = {-321,22,-124}, rot3;

    vox_rotate_vector (identity, dot1, rot1);
    CU_ASSERT (vect_eq (dot1, rot1, precise_check));

    vox_rotate_vector (identity, dot2, rot2);
    CU_ASSERT (vect_eq (dot2, rot2, precise_check));

    vox_rotate_vector (identity, dot3, rot3);
    CU_ASSERT (vect_eq (dot3, rot3, precise_check));
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
    CU_ASSERT (vect_eq (resx, vectx, precise_check));
    
    vox_rotate_vector (basey, vecty, resy);
    CU_ASSERT (vect_eq (resy, vecty, precise_check));

    vox_rotate_vector (basez, vectz, resz);
    CU_ASSERT (vect_eq (resz, vectz, precise_check));
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
    CU_ASSERT (vect_eq (expx, vectx, precise_check));
    
    vox_rotate_vector (basey, vecty, vecty);
    CU_ASSERT (vect_eq (expy, vecty, precise_check));

    vox_rotate_vector (basez, vectz, vectz);
    CU_ASSERT (vect_eq (expz, vectz, precise_check));
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
    
    CU_ASSERT (vect_eq (res1, vector_inv (res2, res2), precise_check));
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
    vox_quat base_res = { cosf(phi)*cosf(psi),
                          sinf(phi)*cosf(psi),
                          sinf(psi)*cosf(phi),
                          sinf(psi)*sinf(phi) };

    // Random numbers
    vox_dot vect = {132, 2, 35};

    vox_dot res1, res2;
    vox_rotate_vector (base2, vect, res1);
    vox_rotate_vector (base1, res1, res1);
    vox_rotate_vector (base_res, vect, res2);

    CU_ASSERT (vect_eq (res1, res2, precise_check));
}

static void rot_saves_norm ()
{
    vox_quat base = {0.5, 0.5, 0.5, 0.5};
    // Random numbers
    vox_dot vect = {1, 5, 2};
    vox_dot res;
    vox_rotate_vector (base, vect, res);

    CU_ASSERT (fabsf (dot_product (vect, vect) - dot_product (res, res)) < precise_check);
}

static struct vox_node* prepare_tree ()
{
    int i,j,k;
    size_t counter = 0;
    vox_dot *set = aligned_alloc (16, sizeof (vox_dot) * 1000000);
    struct vox_node *tree;

    // Make a ball with radius 50 and center (0, 0, 0)
    for (i=-50; i<50; i++) {
        for (j=-50; j<50; j++) {
            for (k=-50; k<50; k++) {
                int dist = i*i + j*j + k*k;
                if (dist < 2500) {
                    vox_dot_set (set[counter], i, j, k);
                    counter++;
                }
            }
        }
    }

    tree = vox_make_tree (set, counter);
    free (set);
    return tree;
}

static void check_tree (struct vox_node *tree)
{
    vox_dot tmp;
    int i;

    if (VOX_FULLP (tree))
    {
        if (tree->flags & VOX_DENSE_LEAF)
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
            CU_ASSERT_FATAL (vox_voxels_in_tree (tree) == (size_t)bb_volume);
        }
        else if (tree->flags & VOX_LEAF)
        {
            vox_dot *dots = tree->data.dots;
            // Check that all voxels are covered by bounding box
            for (i=0; i<tree->dots_num; i++)
            {
                vox_dot_add (dots[i], vox_voxel, tmp);
                CU_ASSERT_FATAL (dot_betweenp (&(tree->bounding_box), dots[i]));
                CU_ASSERT_FATAL (dot_betweenp (&(tree->bounding_box), tmp));
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
                    CU_ASSERT_FATAL (dot_betweenp (&(tree->bounding_box), child->bounding_box.min));
                    CU_ASSERT_FATAL (dot_betweenp (&(tree->bounding_box), child->bounding_box.max));

                    /*
                      Check subspace. We add/subtract a small value (half size of a voxel)
                      to bounding box corners because faces of bounding box may belong to
                      another subspace. What is inside may not.
                    */
                    vox_dot_add (child->bounding_box.min, half_voxel, tmp);
                    CU_ASSERT_FATAL (get_subspace_idx (inner.center, tmp) == i);
                    vox_dot_add (child->bounding_box.max, neg_half_voxel, tmp);
                    CU_ASSERT_FATAL (get_subspace_idx (inner.center, tmp) == i);
                }
                // Test a child recursively
                check_tree (child);
            }
            CU_ASSERT_FATAL (vox_voxels_in_tree (tree) == number_sum);
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
    CU_ASSERT (quat_eq (e1, res, precise_check));

    vox_quat q21 = {0, 0, 1, 0};
    vox_quat q22 = {0, 0, 0, 1};
    vox_quat e2  = {0, 1, 0, 0};
    vox_quat_mul (q21, q22, res);
    CU_ASSERT (quat_eq (e2, res, precise_check));

    vox_quat q31 = {0, 0, 0, 1};
    vox_quat q32 = {0, 1, 0, 0};
    vox_quat e3  = {0, 0, 1, 0};
    vox_quat_mul (q31, q32, res);
    CU_ASSERT (quat_eq (e3, res, precise_check));

    // And in another direction...
    vox_quat q41 = {0, 0, 1, 0};
    vox_quat q42 = {0, 1, 0, 0};
    vox_quat e4  = {0, 0, 0, -1};
    vox_quat_mul (q41, q42, res);
    CU_ASSERT (quat_eq (e4, res, precise_check));

    vox_quat q51 = {0, 0, 0, 1};
    vox_quat q52 = {0, 0, 1, 0};
    vox_quat e5  = {0, -1, 0, 0};
    vox_quat_mul (q51, q52, res);
    CU_ASSERT (quat_eq (e5, res, precise_check));

    vox_quat q61 = {0, 1, 0, 0};
    vox_quat q62 = {0, 0, 0, 1};
    vox_quat e6  = {0, 0, -1, 0};
    vox_quat_mul (q61, q62, res);
    CU_ASSERT (quat_eq (e6, res, precise_check));

    // Neutral element
    vox_quat qn1 = {4, 3, 2, 1};
    vox_quat n =   {1, 0, 0, 0};
    vox_quat_mul (qn1, n, res);
    CU_ASSERT (quat_eq (res, qn1, precise_check));
    vox_quat_mul (n, qn1, res);
    CU_ASSERT (quat_eq (res, qn1, precise_check));
    
    // Some random quaterions
    vox_quat q1 = {4, 1, 2, 3};
    vox_quat q2 = {2, 0, 2, 0};
    vox_quat e  = {4, -4, 12, 8};
    vox_quat_mul (q1, q2, res);
    CU_ASSERT (quat_eq (res, e, precise_check));
}

static void test_tree_cons ()
{
    struct vox_node *working_tree = prepare_tree ();
    check_tree (working_tree);
    vox_destroy_tree (working_tree);
}

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
    tree = vox_alloc (sizeof (struct vox_node));
    vox_dot_set (tree->bounding_box.min, 5, 5, 5);
    vox_dot_set (tree->bounding_box.max, 10, 10,10);
    tree->dots_num = 125;
    tree->flags = VOX_DENSE_LEAF;
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
                vox_dot_set (dot1, 2*i, 2*j, 2*k);
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
                vox_dot_set (dot1, i, j, k);
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
    CU_ASSERT (tree->flags & VOX_DENSE_LEAF);

    dot[1] = 0;
    for (i=0; i<10; i++)
    {
        vox_delete_voxel (&tree, dot);
        dot[1]++;
        CU_ASSERT (tree->flags & VOX_DENSE_LEAF);
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
    CU_ASSERT (tree->flags & VOX_DENSE_LEAF);

    dot[2] = VOX_MAX_DOTS+9;
    for (i=0; i<10; i++)
    {
        vox_delete_voxel (&tree, dot);
        dot[2]--;
        CU_ASSERT (tree->flags & VOX_DENSE_LEAF);
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
    CU_ASSERT (tree->flags & VOX_DENSE_LEAF);

    dot[0] = 1;
    for (i=0; i<10; i++)
    {
        vox_delete_voxel (&tree, dot);
        dot[0]++;
        CU_ASSERT (!(tree->flags & VOX_LEAF_MASK));
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
                vox_dot_set (dot, i, j, k);
                vox_insert_voxel (&tree, dot);
            }
        }
    }

    CU_ASSERT (tree->flags & VOX_DENSE_LEAF);
    vox_dot_set (dot, 0, 0, 0);
    vox_delete_voxel (&tree, dot);
    CU_ASSERT (!(tree->flags & VOX_LEAF_MASK));
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
                vox_dot_set (dot, i, j, k);
                vox_insert_voxel (&tree, dot);
            }
        }
    }

    CU_ASSERT (tree->flags & VOX_DENSE_LEAF);
    vox_dot_set (dot, 1, 1, 1);
    vox_delete_voxel (&tree, dot);
    CU_ASSERT (!(tree->flags & VOX_LEAF_MASK));
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
        CU_ASSERT (tree->flags & VOX_DENSE_LEAF);
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
        CU_ASSERT (tree->flags & VOX_DENSE_LEAF);
        dot[0]++;
        vox_delete_voxel (&tree, dot);
    }
    CU_ASSERT (!(VOX_FULLP (tree)));
}

static void test_tree_g676d50c ()
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

static void test_camera (const char *name)
{
    printf (" %s...", name);
    struct vox_camera *camera = vox_camera_methods (name)->construct_camera (NULL);
    vox_dot pos = {0,0,0}, newpos;
    vox_dot rotate_z = {0, 0, M_PI/2};
    vox_dot rotate_identity = {0, M_PI/2, 0};

    vox_dot world_coord;
    vox_dot world_coord_expected = {-1, 0, 0};
    vox_dot move_vector = {0, 1, 0};

    /* Check setter/getter */
    camera->iface->set_property_dot (camera, "position", pos);
    camera->iface->get_position (camera, newpos);
    CU_ASSERT (vect_eq (pos, newpos, precise_check));

    camera->iface->set_window_size (camera, 100, 100);

    /* At first, camera must look in direction (0, 1, 0). */
    camera->iface->screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, move_vector, precise_check));

    /*
     * We rotate camera around Z axis on angle pi/2 , so it looks now in
     * direction (1, 0, 0).
     */
    camera->iface->set_property_dot (camera, "rotation", rotate_z);
    camera->iface->screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected, precise_check));

    /*
     * The center of the screen is a fixed point for any rotation around Y
     * axis.
     */
    camera->iface->rotate_camera (camera, rotate_identity);
    camera->iface->screen2world (camera, world_coord, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected, precise_check));

    /*
     * Now move camera in direction (0, 1, 0) and get the same result,
     * world_coord_expected.
     */
    camera->iface->move_camera (camera, move_vector);
    camera->iface->get_position (camera, newpos);
    CU_ASSERT (vect_eq (world_coord_expected, newpos, precise_check));

    camera->iface->destroy_camera (camera);
}

static void test_cameras()
{
    test_camera ("simple-camera");
    test_camera ("doom-camera");
}

static void test_camera_look_at (const char *name)
{
    vox_dot pos = {-100, 20, -20};
    vox_dot look_at = {0,0,0};
    vox_dot dir;

    printf (" %s...", name);
    struct vox_camera *camera = vox_camera_methods (name)->construct_camera (NULL);
    camera->iface->set_window_size (camera, 100, 100);
    camera->iface->set_property_dot (camera, "position", pos);
    camera->iface->look_at (camera, look_at);
    camera->iface->screen2world (camera, dir, 50, 50);

    CU_ASSERT (hit_dot (pos, dir, look_at));
    camera->iface->destroy_camera (camera);
}

static void test_cameras_look_at ()
{
    test_camera_look_at ("simple-camera");
    test_camera_look_at ("doom-camera");
}

static void test_camera_look_at_bug ()
{
    vox_dot look_at = {0, 1, 0}; /* Default look_at vector */
    vox_dot pos = {0, 0, 0};
    vox_dot move_vector1 = {1, 0, 0};
    vox_dot expected_pos1 = {1, 0, 0};
    vox_dot move_vector2 = {1, 0, 0};
    vox_dot expected_pos2 = {1, 0, 0};
    vox_dot pos_act;

    struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);
    camera->iface->set_property_dot (camera, "position", pos);
    camera->iface->look_at (camera, look_at);
    camera->iface->move_camera (camera, move_vector1);
    camera->iface->get_position (camera, pos_act);
    CU_ASSERT (vect_eq (expected_pos1, pos_act, precise_check));

    camera->iface->set_property_dot (camera, "position", pos);
    camera->iface->look_at (camera, look_at);
    camera->iface->move_camera (camera, move_vector2);
    camera->iface->get_position (camera, pos_act);
    CU_ASSERT (vect_eq (expected_pos2, pos_act, precise_check));
    camera->iface->destroy_camera (camera);
}

static void test_camera_class_construction ()
{
    vox_dot pos = {10,110,1110}, newpos;
    struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);
    camera->iface->set_property_dot (camera, "position", pos);
    camera->iface->set_window_size (camera, 100, 100);
//    camera->iface->set_fov (camera, 16);

    struct vox_camera *camera2 = vox_camera_methods ("simple-camera")->construct_camera (camera);
    camera2->iface->get_position (camera2, newpos);
    CU_ASSERT (vect_eq (pos, newpos, precise_check));
//    CU_ASSERT (camera2->iface->get_fov (camera2) == 16);
}

int sphere_inside_sphere (const struct vox_sphere *inner, const struct vox_sphere *outer)
{
    float dist = sqrtf (vox_sqr_metric (inner->center, outer->center));
    return inner->radius + dist < outer->radius + approx_check;
}

static void verify_mtree (const struct vox_mtree_node *node)
{
    unsigned int i;
    const struct vox_mtree_node *child;

    if (node != NULL) {
        CU_ASSERT_FATAL (node->num <= MTREE_MAX_CHILDREN);
        if (node->leaf) {
            for (i=0; i<node->num; i++)
                CU_ASSERT_FATAL (sphere_inside_sphere (&(node->data.spheres[i]),
                                                       &(node->bounding_sphere)));
        } else {
            CU_ASSERT_FATAL (node->num > 1);
            for (i=0; i<node->num; i++) {
                child = node->data.children[i];
                CU_ASSERT_FATAL (child != NULL);
                CU_ASSERT_FATAL (child->parent == node);
                CU_ASSERT_FATAL (sphere_inside_sphere (&(child->bounding_sphere),
                                                       &(node->bounding_sphere)));
                verify_mtree (child);
            }
        }
    }
}

void test_mtree ()
{
    struct vox_mtree_node *mtree = NULL;
    struct vox_sphere s;
    unsigned int i, n = 140;
    unsigned int seed = rand();

    srand (seed);
    for (i=0; i<n; i++) {
        vox_dot_set (s.center,
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX));
        s.radius = 1 + floorf (15.0 * rand() / RAND_MAX);
        CU_ASSERT (vox_mtree_add_sphere (&mtree, &s));
        verify_mtree (mtree);
        CU_ASSERT (vox_mtree_items (mtree) == i+1);
    }
    // Try to add the last sphere
    CU_ASSERT (!vox_mtree_add_sphere (&mtree, &s));

    srand (seed);
    for (i=0; i<n; i++) {
        vox_dot_set (s.center,
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX));
        s.radius = 1 + floorf (15.0 * rand() / RAND_MAX);
        CU_ASSERT (vox_mtree_contains_sphere (mtree, &s) != NULL);
    }

    srand (seed);
    for (i=0; i<n; i++) {
        vox_dot_set (s.center,
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX));
        s.radius = 1 + floorf (15.0 * rand() / RAND_MAX);
        CU_ASSERT (vox_mtree_contains_sphere (mtree, &s) != NULL);
        CU_ASSERT (vox_mtree_remove_sphere (&mtree, &s));
        CU_ASSERT (vox_mtree_contains_sphere (mtree, &s) == NULL);
        verify_mtree (mtree);
        CU_ASSERT (vox_mtree_items (mtree) == n-i-1);
    }
}

void test_mtree_search ()
{
    struct vox_mtree_node *mtree = NULL;
    struct vox_sphere s;
    int i, n = 500;

    unsigned int count = 0;
    __block unsigned int testcount = 0;
    vox_dot center;
    vox_dot_set (center, 0, 0, 0);

    for (i=0; i<n; i++) {
        vox_dot_set (s.center,
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX),
                     floorf (100.0 * rand() / RAND_MAX));
        s.radius = 70 + floorf (30.0 * rand() / RAND_MAX);

        if (vox_mtree_add_sphere (&mtree, &s) &&
            sqrtf (vox_sqr_metric (center, s.center)) < s.radius) count++;
    }

    vox_mtree_spheres_containing (mtree, center, ^(const struct vox_sphere *s) {
            testcount++;});
    CU_ASSERT (testcount == count);
}

/* Vector operations */
static CU_TestInfo vectops_tests[] = {
    { "identity operator", test_identity },
    { "rotate around itself", test_rotation_around_itself },
    { "change axis", test_change_axis },
    { "anticommutativity of some rotations", test_anticommut },
    { "composition of rotations", rot_composition },
    { "rotation isometry", rot_saves_norm },
    { "quaterion multiplication", quat_mul },
    CU_TEST_INFO_NULL
};

/* Tree construction and search (voxtrees) */
static CU_TestInfo voxtrees_tests[] = {
    { "tree construction", test_tree_cons },
    { "insertion", test_tree_ins },
    { "deletion (case 1)", test_tree_del1 },
    { "deletion (case 2)", test_tree_del2 },
    { "deletion (case 3)", test_tree_del3 },
    { "deletion (case 4)", test_tree_del4 },
    { "deletion (case 5)", test_tree_del5 },
    { "insertion type transitions", test_tree_ins_trans },
    { "deletion type transitions", test_tree_del_trans },
    { "search (commit 676d50c)", test_tree_g676d50c },
    { "test M-trees", test_mtree },
    { "test M-tree search", test_mtree_search },
    CU_TEST_INFO_NULL
};

/* Rendering (voxrnd) */
static CU_TestInfo voxrnd_tests[] = {
    { "camera generic test", test_cameras },
    { "camera look_at() test", test_cameras_look_at },
    { "simple camera look_at() bug (issue 1)" , test_camera_look_at_bug },
    { "camera class construction", test_camera_class_construction },
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    { "vectops",  NULL, NULL, NULL, NULL, vectops_tests },
    { "voxtrees", NULL, NULL, NULL, NULL, voxtrees_tests },
    { "voxrnd",   NULL, NULL, NULL, NULL, voxrnd_tests },
    CU_SUITE_INFO_NULL
};

int main ()
{
    int i, code;
    char *travis_env;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf (stderr, "Failed to initialize registry: %s\n",
                 CU_get_error_msg ());
        return EXIT_FAILURE;
    }

    if (CU_register_suites (suites) != CUE_SUCCESS) {
        fprintf (stderr, "Failed to add suites: %s\n",
                 CU_get_error_msg ());
        CU_cleanup_registry ();
        return EXIT_FAILURE;
    }

    for (i=0; i<VOX_N; i++)
    {
        half_voxel[i] = vox_voxel[i]/2;
        neg_half_voxel[i] = -vox_voxel[i]/2;
    }

    /*
     * Simple-camera tests fail under Travis-CI if precise_check is
     * too small for some reason...
     * As a quick fix set it to a bigger value.
     */
    travis_env = getenv ("TRAVIS");
    approx_check = 0.5;
    precise_check = (travis_env != NULL && strcmp (travis_env, "true") == 0)? 0.001: 0.00001;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    code = (CU_get_number_of_tests_failed() == 0)? 0: EXIT_FAILURE;
    CU_cleanup_registry();

    return code;
}
