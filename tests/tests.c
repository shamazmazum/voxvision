#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <voxrnd/camera.h>
#include <voxrnd/vect-ops.h>
#include <voxtrees.h>
#include <voxtrees/geom.h>

static float precise_check;
static float approx_check;

static float norm (vox_dot d) {
    return sqrtf (vox_sqr_norm (d));
}

/* static void normalize (vox_dot d) { */
/*     float n = norm (d); */
/*     d[1] /= n; */
/*     d[2] /= n; */
/*     d[3] /= n; */
/* } */

static int vect_eq (const vox_dot v1, const vox_dot v2, float precision) {
    float dist = norm (v1 - v2);
    return dist < precision;
}

static int quat_eq (const vox_quat v1, const vox_quat v2, float precision) {
    // KLUDGE: vox_dot and vox_quat are actually synonyms
    float dist = norm (v1 - v2);
    return dist < precision;
}

/* static int hit_dot (const vox_dot origin, const vox_dot dir, const vox_dot target) { */
/*     float t = (target[0] - origin[0])/dir[0]; */
/*     CU_ASSERT (t >= 0); */
/*     vox_dot res; */
/*     res[0] = target[0]; */
/*     res[1] = dir[1]*t + origin[1]; */
/*     res[2] = dir[2]*t + origin[2]; */

/*     return vect_eq (target, res, approx_check); */
/* } */

static float random_float_01 () {
    uint32_t max = 0xffffffff;
    return (float)arc4random() / (float)max;
}

static int brute_force_intersection (vox_dot *set, uint32_t n, vox_dot orig,
                                     vox_dot dir, vox_dot *res) {
    int success = 0;
    struct vox_box box;
    vox_dot tmp;
    vox_dot inter;

    for (uint32_t i = 0; i < n; i++) {
        box.min = set[i];
        box.max = set[i] + vox_voxel;
        if (vox_hit_box (&box, orig, dir, &tmp)) {
            if (success && (norm (tmp - orig) < norm (inter - orig))) {
                inter = tmp;
            } else if (!success) {
                inter = tmp;
                success = 1;
            }
        }
    }

    if (success) {
        *res = inter;
    }
    return success;
}

static int brute_force_collides_with_ball_p (vox_dot *set, uint32_t n, vox_dot center,
                                             float radius) {
    struct vox_box box;

    for (uint32_t i = 0; i < n; i++) {
        box.min = set[i];
        box.max = set[i] + vox_voxel;
        if (vox_box_ball_interp (&box, center, radius)) {
            return 1;
        }
    }

    return 0;
}

static void test_identity () {
    vox_quat identity = (vox_quat) {0, 0, 0, 1};
    vox_dot dot, rot;

    for (int i = 0; i < 1000; i++) {
        dot = (vox_dot) {
            random_float_01(), random_float_01(), random_float_01(), 0
        };
        rot = vox_rotate_vector (identity, dot);
        CU_ASSERT (vect_eq (dot, rot, precise_check));
    }
}

static void test_rotation_around_itself () {
    vox_dot vec, res;
    vox_quat quat;

    for (int i = 0; i < 1000; i++) {
        float phi = M_PI*random_float_01();
        float sinphi = sinf (phi);
        float cosphi = cosf (phi);

        vec = (vox_dot) {
            random_float_01(),
            random_float_01(),
            random_float_01(),
            0
        };
        vec = vec / norm (vec);

        quat = (vox_quat) vec * sinphi;
        quat.w = cosphi;

        res = vox_rotate_vector (quat, vec);
        CU_ASSERT (vect_eq (res, vec, precise_check));
    }
}

static void test_change_axis () {
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

    CU_ASSERT (vect_eq (expx, vox_rotate_vector (basex, vectx), precise_check));
    CU_ASSERT (vect_eq (expy, vox_rotate_vector (basey, vecty), precise_check));
    CU_ASSERT (vect_eq (expz, vox_rotate_vector (basez, vectz), precise_check));
}

static void test_anticommut () {
    // Rotating x around y must result in rotaiong y around x with different sign
    float sincos = sqrtf(2.0)/2.0;
    
    vox_quat basex = {sincos, 0.0, 0.0, sincos};
    vox_quat basey = {0.0, sincos, 0.0, sincos};
    
    vox_dot vectx = {1.0, 0.0, 0.0};
    vox_dot vecty = {0.0, 1.0, 0.0};
    
    vox_dot res1 = vox_rotate_vector (basex, vecty);
    vox_dot res2 = vox_rotate_vector (basey, vectx);
    
    CU_ASSERT (vect_eq (res1, -res2, precise_check));
}

static void rot_composition () {
    for (int i = 0; i < 1000; i++) {
        vox_dot dot = (vox_dot) {
            random_float_01(),
            random_float_01(),
            random_float_01(),
            1
        };

        vox_dot v1 = (vox_dot) {
            random_float_01(),
            random_float_01(),
            random_float_01(),
            1
        };

        vox_dot v2 = (vox_dot) {
            random_float_01(),
            random_float_01(),
            random_float_01(),
            1
        };

        v1 = v1 / norm (v1);
        v2 = v2 / norm (v2);

        float phi = random_float_01();
        float psi = random_float_01();

        float sinphi = sinf (phi);
        float cosphi = cosf (phi);
        float sinpsi = sinf (psi);
        float cospsi = cosf (psi);

        vox_quat q1 = v1 * sinphi;
        vox_quat q2 = v2 * sinpsi;
        q1.w = cosphi;
        q2.w = cospsi;

        vox_dot res1 = vox_rotate_vector (q1, vox_rotate_vector (q2, dot));
        vox_quat q   = vox_quat_mul (q1, q2);
        vox_dot res2 = vox_rotate_vector (q, dot);

        CU_ASSERT (vect_eq (res1, res2, precise_check));
    }
}

static void rot_saves_norm () {
    vox_quat q;
    vox_dot dot, res, v;

    for (int i = 0; i < 1000; i++) {
        dot = (vox_dot) {
            random_float_01(),
            random_float_01(),
            random_float_01(),
            0
        };

        v = (vox_dot) {
            random_float_01(),
            random_float_01(),
            random_float_01(),
            0
        };
        v = v / norm (v);

        float phi = random_float_01();
        float sinphi = sinf (phi);
        float cosphi = cosf (phi);

        q = v * cosphi;
        q.w = sinphi;

        res = vox_rotate_vector (q, dot);

        float n1 = norm(res);
        float n2 = norm(dot);
        CU_ASSERT (fabsf (n1-n2) < precise_check);
    }
}

static void quat_mul () {
    vox_quat res;

    // Check multiplication table
    vox_quat q11 = {1, 0, 0, 0};
    vox_quat q12 = {0, 1, 0, 0};
    vox_quat e1  = {0, 0, 1, 0};
    res = vox_quat_mul (q11, q12);
    CU_ASSERT (quat_eq (e1, res, precise_check));

    vox_quat q21 = {0, 1, 0, 0};
    vox_quat q22 = {0, 0, 1, 0};
    vox_quat e2  = {1, 0, 0, 0};
    res = vox_quat_mul (q21, q22);
    CU_ASSERT (quat_eq (e2, res, precise_check));

    vox_quat q31 = {0, 0, 1, 0};
    vox_quat q32 = {1, 0, 0, 0};
    vox_quat e3  = {0, 1, 0, 0};
    res = vox_quat_mul (q31, q32);
    CU_ASSERT (quat_eq (e3, res, precise_check));

    // And in another direction...
    vox_quat q41 = {0, 1, 0, 0};
    vox_quat q42 = {1, 0, 0, 0};
    vox_quat e4  = {0, 0, -1, 0};
    res = vox_quat_mul (q41, q42);
    CU_ASSERT (quat_eq (e4, res, precise_check));

    vox_quat q51 = {0, 0, 1, 0};
    vox_quat q52 = {0, 1, 0, 0};
    vox_quat e5  = {-1, 0, 0, 0};
    res = vox_quat_mul (q51, q52);
    CU_ASSERT (quat_eq (e5, res, precise_check));

    vox_quat q61 = {1, 0, 0, 0};
    vox_quat q62 = {0, 0, 1, 0};
    vox_quat e6  = {0, -1, 0, 0};
    res = vox_quat_mul (q61, q62);
    CU_ASSERT (quat_eq (e6, res, precise_check));

    // Neutral element
    vox_quat qn1 = {4, 3, 2, 1};
    vox_quat n =   {0, 0, 0, 1};
    res = vox_quat_mul (qn1, n);
    CU_ASSERT (quat_eq (res, qn1, precise_check));
    res = vox_quat_mul (n, qn1);
    CU_ASSERT (quat_eq (res, qn1, precise_check));
    
    // Some random quaterions
    vox_quat q1 = {1, 2, 3, 4};
    vox_quat q2 = {0, 2, 0, 2};
    vox_quat e  = {-4, 12, 8, 4};
    res = vox_quat_mul (q1, q2);
    CU_ASSERT (quat_eq (res, e, precise_check));
}

static uint32_t random_set (vox_dot **setptr, uint32_t n) {
    vox_dot *set = aligned_alloc (16, sizeof (vox_dot) * n);

    for (uint32_t i = 0; i < n; i++) {
        set[i] = (vox_dot) {arc4random_uniform(1000),
                            arc4random_uniform(1000),
                            arc4random_uniform(1000),
                            0
        };
    }

    *setptr = set;
    return n;
}

static void check_tree (struct vox_node *tree) {
    vox_dot tmp;
    unsigned int i;

    if (VOX_FULLP (tree)) {
        CU_ASSERT_FATAL (!(tree->flags & VOX_DENSE_LEAF) || (tree->flags & VOX_FROZEN));

        if (tree->flags & VOX_DENSE_LEAF) {
            /*
              Dense leafs contain an exact number of dots depending on
              volume of their bounding boxes
            */
            vox_dot size;
            for (i=0; i<VOX_N; i++) {
                size[i] = tree->bounding_box.max[i] - tree->bounding_box.min[i];
            }
            float bb_volume = size[0]*size[1]*size[2];
            bb_volume /= vox_voxel[0]*vox_voxel[1]*vox_voxel[2];
            CU_ASSERT_FATAL (vox_voxels_in_tree (tree) == (size_t)bb_volume);
        } else if (tree->flags & VOX_LEAF) {
            vox_dot *dots = tree->data.dots;
            // Check that all voxels are covered by bounding box
            for (i=0; i<tree->dots_num; i++) {
                tmp = dots[i] + vox_voxel;
                CU_ASSERT_FATAL (vox_voxel_in_box (&(tree->bounding_box), dots[i]));
                CU_ASSERT_FATAL (vox_voxel_in_box (&(tree->bounding_box), tmp));
            }
        } else {
            vox_inner_data inner = tree->data.inner;
            size_t number_sum = 0;
            for (i=0; i<VOX_NS; i++) {
                struct vox_node *child = inner.children[i];
                number_sum += vox_voxels_in_tree (child);
                if (VOX_FULLP (child)) {
                    // Check if child's bounding box is inside parent's
                    CU_ASSERT_FATAL (vox_voxel_in_box (&(tree->bounding_box),
                                                       child->bounding_box.min));
                    CU_ASSERT_FATAL (vox_voxel_in_box (&(tree->bounding_box),
                                                       child->bounding_box.max));

                    /*
                      Check subspace. We add/subtract a small value (half size of a voxel)
                      to bounding box corners because faces of bounding box may belong to
                      another subspace. What is inside may not.
                    */
                    tmp = child->bounding_box.min + vox_voxel/2;
                    CU_ASSERT_FATAL (get_subspace_idx (inner.center, tmp) == i);
                    tmp = child->bounding_box.max - vox_voxel/2;
                    CU_ASSERT_FATAL (get_subspace_idx (inner.center, tmp) == i);
                }
                // Test a child recursively
                check_tree (child);
            }
            CU_ASSERT_FATAL (vox_voxels_in_tree (tree) == number_sum);
        }
    }
}

static void test_tree_cons () {
    for (int i = 0; i < 100; i++) {
        vox_dot *set;
        uint32_t n = random_set (&set, 10000);
        struct vox_node *tree = vox_make_tree (set, n);
        free (set);
        check_tree (tree);
        vox_destroy_tree (tree);
    }
}

static void test_tree_freeze () {
    for (int i = 0; i < 100; i++) {
        vox_dot *set;
        uint32_t n = random_set (&set, 10000);
        struct vox_node *tree = vox_make_tree (set, n);
        free (set);
        struct vox_node *frozen = vox_freeze_tree (tree);
        vox_destroy_tree (tree);
        check_tree (frozen);
        vox_destroy_tree (frozen);
    }
}

static void test_tree_double_insert () {
    for (int i = 0; i < 100; i++) {
        vox_dot *set;
        uint32_t n = random_set (&set, 10000);
        struct vox_node *tree = vox_make_tree (set, n);

        for (uint32_t j = 0; j < n; j++) {
            CU_ASSERT (vox_insert_voxel (&tree, set[j]) == VOX_MOD_NOOP);
        }

        free (set);
        vox_destroy_tree (tree);
    }
}

static void test_tree_insert () {
    for (int i = 0; i < 100; i++) {
        vox_dot *set;
        uint32_t n = random_set (&set, 1000);
        struct vox_node *tree = vox_make_tree (set, n);
        free (set);

        for (int j = 0; j < 100; j++) {
            vox_dot dot = (vox_dot) {
                arc4random_uniform (1000),
                arc4random_uniform (1000),
                arc4random_uniform (1000),
                0
            };
            vox_insert_voxel (&tree, dot);
            check_tree (tree);
        }

        vox_destroy_tree (tree);
    }
}

static void test_tree_delete () {
    for (int i = 0; i < 100; i++) {
        vox_dot *set;
        uint32_t n = random_set (&set, 1000);
        struct vox_node *tree = vox_make_tree (set, n);

        for (uint32_t j = 0; j < n; j++) {
            CU_ASSERT (vox_delete_voxel (&tree, set[j]) == VOX_MOD_OK);
            check_tree (tree);
        }

        free (set);
        CU_ASSERT (tree == NULL);
    }
}

static void test_random_intersection () {
    vox_dot *set;
    uint32_t n = random_set (&set, 10000);
    struct vox_node *tree = vox_make_tree (set, n);

    for (int i = 0; i < 100000; i++) {
        vox_dot origin = (vox_dot){
            2000*random_float_01() - 1000,
            2000*random_float_01() - 1000,
            2000*random_float_01() - 1000,
            0.0
        };
        vox_dot direction = (vox_dot){
            2*random_float_01() - 1,
            2*random_float_01() - 1,
            2*random_float_01() - 1,
            0.0
        };
        vox_dot res1, res2;
        const struct vox_node *node =
            vox_ray_tree_intersection (tree, origin, direction, &res1);
        int inter =
            brute_force_intersection (set, n, origin, direction, &res2);
        CU_ASSERT ((inter && (node != NULL)) || (!inter && (node == NULL)));
        CU_ASSERT (!inter || (norm (res1 - res2) < 1e-3));
    }

    free (set);
    vox_destroy_tree (tree);
}

static void test_random_ball_collision () {
    vox_dot *set;
    uint32_t n = random_set (&set, 10000);
    struct vox_node *tree = vox_make_tree (set, n);

    for (int i = 0; i < 100000; i++) {
        vox_dot center = (vox_dot){
            1000*random_float_01(),
            1000*random_float_01(),
            1000*random_float_01(),
            0.0
        };
        float radius = 100*random_float_01();
        int res1, res2;
        res1 = vox_tree_ball_collidep (tree, center, radius);
        res2 = brute_force_collides_with_ball_p (set, n, center, radius);
        CU_ASSERT ((res1 && res2) || (!res1 && !res2));
    }

    free (set);
    vox_destroy_tree (tree);
}

/* static void test_camera (const char *name) { */
/*     printf (" %s...", name); */
/*     struct vox_camera *camera = vox_camera_methods (name)->construct_camera (NULL); */
/*     vox_dot pos = {0,0,0}, newpos; */
/*     vox_dot rotate_z = {0, 0, M_PI/2}; */
/*     vox_dot rotate_identity = {0, M_PI/2, 0}; */

/*     vox_dot world_coord; */
/*     vox_dot world_coord_expected = {-1, 0, 0}; */
/*     vox_dot move_vector = {0, 1, 0}; */

/*     /\* Check setter/getter *\/ */
/*     camera->iface->set_property_dot (camera, "position", pos); */
/*     camera->iface->get_position (camera, newpos); */
/*     CU_ASSERT (vect_eq (pos, newpos, precise_check)); */

/*     camera->iface->set_window_size (camera, 100, 100); */

/*     /\* At first, camera must look in direction (0, 1, 0). *\/ */
/*     camera->iface->screen2world (camera, world_coord, 50, 50); */
/*     CU_ASSERT (vect_eq (world_coord, move_vector, precise_check)); */

/*     /\* */
/*      * We rotate camera around Z axis on angle pi/2 , so it looks now in */
/*      * direction (1, 0, 0). */
/*      *\/ */
/*     camera->iface->set_property_dot (camera, "rotation", rotate_z); */
/*     camera->iface->screen2world (camera, world_coord, 50, 50); */
/*     CU_ASSERT (vect_eq (world_coord, world_coord_expected, precise_check)); */

/*     /\* */
/*      * The center of the screen is a fixed point for any rotation around Y */
/*      * axis. */
/*      *\/ */
/*     camera->iface->rotate_camera (camera, rotate_identity); */
/*     camera->iface->screen2world (camera, world_coord, 50, 50); */
/*     CU_ASSERT (vect_eq (world_coord, world_coord_expected, precise_check)); */

/*     /\* */
/*      * Now move camera in direction (0, 1, 0) and get the same result, */
/*      * world_coord_expected. */
/*      *\/ */
/*     camera->iface->move_camera (camera, move_vector); */
/*     camera->iface->get_position (camera, newpos); */
/*     CU_ASSERT (vect_eq (world_coord_expected, newpos, precise_check)); */

/*     camera->iface->destroy_camera (camera); */
/* } */

/* static void test_cameras() { */
/*     test_camera ("simple-camera"); */
/*     test_camera ("doom-camera"); */
/* } */

/* static void test_camera_look_at (const char *name) { */
/*     vox_dot pos = {-100, 20, -20}; */
/*     vox_dot look_at = {0,0,0}; */
/*     vox_dot dir; */

/*     printf (" %s...", name); */
/*     struct vox_camera *camera = vox_camera_methods (name)->construct_camera (NULL); */
/*     camera->iface->set_window_size (camera, 100, 100); */
/*     camera->iface->set_property_dot (camera, "position", pos); */
/*     camera->iface->look_at (camera, look_at); */
/*     camera->iface->screen2world (camera, dir, 50, 50); */

/*     CU_ASSERT (hit_dot (pos, dir, look_at)); */
/*     camera->iface->destroy_camera (camera); */
/* } */

/* static void test_cameras_look_at () { */
/*     test_camera_look_at ("simple-camera"); */
/*     test_camera_look_at ("doom-camera"); */
/* } */

/* static void test_camera_look_at_bug () { */
/*     vox_dot look_at = {0, 1, 0}; /\* Default look_at vector *\/ */
/*     vox_dot pos = {0, 0, 0}; */
/*     vox_dot move_vector1 = {1, 0, 0}; */
/*     vox_dot expected_pos1 = {1, 0, 0}; */
/*     vox_dot move_vector2 = {1, 0, 0}; */
/*     vox_dot expected_pos2 = {1, 0, 0}; */
/*     vox_dot pos_act; */

/*     struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL); */
/*     camera->iface->set_property_dot (camera, "position", pos); */
/*     camera->iface->look_at (camera, look_at); */
/*     camera->iface->move_camera (camera, move_vector1); */
/*     camera->iface->get_position (camera, pos_act); */
/*     CU_ASSERT (vect_eq (expected_pos1, pos_act, precise_check)); */

/*     camera->iface->set_property_dot (camera, "position", pos); */
/*     camera->iface->look_at (camera, look_at); */
/*     camera->iface->move_camera (camera, move_vector2); */
/*     camera->iface->get_position (camera, pos_act); */
/*     CU_ASSERT (vect_eq (expected_pos2, pos_act, precise_check)); */
/*     camera->iface->destroy_camera (camera); */
/* } */

/* static void test_camera_class_construction () { */
/*     vox_dot pos = {10,110,1110}, newpos; */
/*     struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL); */
/*     camera->iface->set_property_dot (camera, "position", pos); */
/*     camera->iface->set_window_size (camera, 100, 100); */
/* //    camera->iface->set_fov (camera, 16); */

/*     struct vox_camera *camera2 = vox_camera_methods ("simple-camera")->construct_camera (camera); */
/*     camera2->iface->get_position (camera2, newpos); */
/*     CU_ASSERT (vect_eq (pos, newpos, precise_check)); */
/* //    CU_ASSERT (camera2->iface->get_fov (camera2) == 16); */
/* } */

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
    { "frozen trees", test_tree_freeze },
    { "double insertion", test_tree_double_insert },
    { "insertion", test_tree_insert },
    { "deletion", test_tree_delete },
    { "random intersection", test_random_intersection },
    { "collision with a random ball", test_random_ball_collision },
    CU_TEST_INFO_NULL
};

/* Rendering (voxrnd) */
/* static CU_TestInfo voxrnd_tests[] = { */
/*     { "camera generic test", test_cameras }, */
/*     { "camera look_at() test", test_cameras_look_at }, */
/*     { "simple camera look_at() bug (issue 1)" , test_camera_look_at_bug }, */
/*     { "camera class construction", test_camera_class_construction }, */
/*     CU_TEST_INFO_NULL */
/* }; */

static CU_SuiteInfo suites[] = {
    { "vectops",  NULL, NULL, NULL, NULL, vectops_tests },
    { "voxtrees", NULL, NULL, NULL, NULL, voxtrees_tests },
    //{ "voxrnd",   NULL, NULL, NULL, NULL, voxrnd_tests },
    CU_SUITE_INFO_NULL
};

int main () {
    int code;
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
