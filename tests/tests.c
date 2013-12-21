#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../src/voxrnd/voxrnd.h"
#include "../src/voxrnd/vect-ops.h"
#include "../src/voxtrees/voxtrees.h"
#include "../src/voxtrees/geom.h"

#define PROC_SUIT_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a suite\n");        \
        goto regcleanup;}                          \
    while (0)

#define PROC_TEST_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a test\n");         \
        goto regcleanup;}                          \
    while (0)

#define PREC 0.0001
#define NSET 2000000

vox_dot *working_set;
struct vox_node *working_tree;

int vect_eq (vox_dot v1, vox_dot v2)
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

int quat_eq (vox_quat v1, vox_quat v2)
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

void test_rotation_around_itself ()
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

void test_change_axis ()
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

void test_anticommut ()
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
    
    CU_ASSERT (vect_eq (res1, vox_vector_inv (res2, res2)));
}

void rot_composition ()
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

void rot_saves_norm ()
{
    vox_quat base = {0.5, 0.5, 0.5, 0.5};
    // Random numbers
    vox_dot vect = {1, 5, 2};
    vox_dot res;
    vox_rotate_vector (base, vect, res);

    CU_ASSERT (fabsf (vox_dot_product (vect, vect) - vox_dot_product (res, res)) < PREC);
}

void prepare_rnd_set_and_tree (struct vox_node **node_ptr, vox_dot **set_ptr)
{
    int i;
    vox_dot *set = malloc (sizeof (vox_dot) * NSET);
    for (i=0; i<NSET; i++)
    {
        set[i][0] = 1.0*rand();
        set[i][1] = 1.0*rand();
        set[i][2] = 1.0*rand();
    }

    *node_ptr = vox_make_tree (set, NSET);
    *set_ptr = set;
}

void check_tree (struct vox_node *tree)
{
    vox_dot snd_corner;
    vox_uint i;
    
    if (VOX_LEAFP (tree))
    {
        vox_leaf_data leaf = tree->data.leaf;
        
        // Check if empty leaf is really empty
        if (leaf.dots_num == 0) CU_ASSERT (!(VOX_FULLP (tree)));
        // Check that all voxels are covered by bounding box
        for (i=0; i<leaf.dots_num; i++)
        {
            sum_vector (leaf.dots[i], vox_voxel, snd_corner);
            CU_ASSERT (dot_betweenp (tree->bb_min, tree->bb_max, leaf.dots[i]));
            CU_ASSERT (dot_betweenp (tree->bb_min, tree->bb_max, snd_corner));
        }
    }
    else
    {
        vox_inner_data inner = tree->data.inner;
        for (i=0; i<VOX_NS; i++)
        {
            struct vox_node *child = inner.children[i];
            if (VOX_FULLP (child))
            {
                // Check if child's bounding box is inside parent's
                CU_ASSERT (dot_betweenp (tree->bb_min, tree->bb_max, child->bb_min));
                CU_ASSERT (dot_betweenp (tree->bb_min, tree->bb_max, child->bb_max));

                // Check subspace
                CU_ASSERT (get_subspace_idx (inner.center, child->bb_min) == i);
                CU_ASSERT (get_subspace_idx (inner.center, child->bb_max) == i);
            }
            // Test a child recursively
            check_tree (child);
        }
    }
}

void quat_mul ()
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

void test_tree_cons () {check_tree (working_tree);}

void test_simp_camera ()
{
    vox_simple_camera cam;
    vox_dot pos = {0,0,0};
    float fov = 1.2;
    class_t *obj = (class_t*)&cam;
    vox_make_simple_camera (&cam, fov, pos);

    CU_ASSERT (memcmp (pos, vox_camera_position_ptr (&cam), sizeof(vox_dot)) == 0);
    CU_ASSERT (fov == GETTER_NAME(fov) (obj));

    fov = 1.3;
    SETTER_NAME(fov) (obj, fov);
    CU_ASSERT (GETTER_NAME(fov) (obj) == fov);

    float phi = 0.2;
    float psi = 0.1;
    SETTER_NAME(phi) (obj, phi);
    SETTER_NAME(psi) (obj, psi);
    CU_ASSERT (GETTER_NAME(phi) (obj) == phi);
    CU_ASSERT (GETTER_NAME(psi) (obj) == psi);

    SETTER_NAME(phi) (obj, M_PI/4);
    SETTER_NAME(psi) (obj, M_PI/4);
    vox_dot world_coord;
    vox_dot world_coord_expected = {0, 0, 400};
    vox_camera_screen2world (obj, world_coord, 100, 100, 50, 50);
    CU_ASSERT (vect_eq (world_coord, world_coord_expected));
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

    // Renderer library
    CU_pSuite rnd_suite = CU_add_suite ("voxrnd", NULL, NULL);
    if (vox_suite == NULL) PROC_SUIT_ERROR;

    test = CU_add_test (rnd_suite, "Camera class", test_simp_camera);
    if (test == NULL) PROC_TEST_ERROR;
    

    printf ("Creating working set and working tree\n");
    prepare_rnd_set_and_tree (&working_tree, &working_set);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
regcleanup: CU_cleanup_registry();
exit_:    return res;
}
