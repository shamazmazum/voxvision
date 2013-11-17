#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../src/vect-ops/vect-ops.h"

#define PROC_SUIT_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a suite\n");        \
        goto regcleanup;}                          \
    while (0)

#define PROC_TEST_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a test\n");         \
        goto regcleanup;}                          \
    while (0)

#define PREC 0.00001

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
    
    rotate_vector (basex, vectx, resx);
    CU_ASSERT (vect_eq (resx, vectx));
    
    rotate_vector (basey, vecty, resy);
    CU_ASSERT (vect_eq (resy, vecty));

    rotate_vector (basez, vectz, resz);
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
    
    rotate_vector (basex, vectx, vectx);
    CU_ASSERT (vect_eq (expx, vectx));
    
    rotate_vector (basey, vecty, vecty);
    CU_ASSERT (vect_eq (expy, vecty));

    rotate_vector (basez, vectz, vectz);
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
    rotate_vector (basex, vecty, res1);
    rotate_vector (basey, vectx, res2);
    
    CU_ASSERT (vect_eq (res1, vector_inv (res2, res2)));
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
    rotate_vector (base2, vect, res1);
    rotate_vector (base1, res1, res1);
    rotate_vector (base_res, vect, res2);

    CU_ASSERT (vect_eq (res1, res2));
}

void rot_saves_norm ()
{
    vox_quat base = {0.5, 0.5, 0.5, 0.5};
    // Random numbers
    vox_dot vect = {1, 5, 2};
    vox_dot res;
    rotate_vector (base, vect, res);

    CU_ASSERT (fabsf (dot_product (vect, vect) - dot_product (res, res)) < PREC);
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

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
regcleanup: CU_cleanup_registry();
exit_:    return res;
}
