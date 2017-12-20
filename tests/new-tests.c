#include <sys/param.h>
#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>
#include <stdio.h>

#include <voxtrees-ng/datareader.h>
#include <voxtrees-ng/tree.h>
#include <voxtrees-ng/geom.h>

#define PROC_SUIT_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a suite\n");        \
        goto regcleanup;}                          \
    while (0)

#define PROC_TEST_ERROR do {res = CU_get_error (); \
        printf ("Failed to add a test\n");         \
        goto regcleanup;}                          \
    while (0)

static struct vox_map_3d *map = NULL;
static struct vox_node *tree = NULL;

static void test_data_loading ()
{
    char name[MAXPATHLEN];
    sprintf (name, "%s/skull.dat", DATAPATH);
    unsigned int dim[3] = {256,256,256};
    const char *error;
    map = vox_read_raw_data (name, dim, 1, ^(unsigned int sample){return sample >= 40;}, &error);
    CU_ASSERT (map != NULL);
}

static void test_tree_cons ()
{
    tree = vox_make_tree (map);
    CU_ASSERT (map != NULL);
}

static unsigned int voxels_in_map (struct vox_map_3d *map)
{
    int i,j,k, value;
    unsigned int count = 0, p = 0;
    for (i=0; i<map->dim[0]; i++) {
        for (j=0; j<map->dim[1]; j++) {
            for (k=0; k<map->dim[2]; k++) {
                value = !!map->map[p];
                if (value) count++;
                p++;
            }
        }
    }
    return count;
}

static void test_num_vox ()
{
    CU_ASSERT (voxels_in_map (map) == vox_voxels_in_tree (tree));
}

static void verify_structure (struct vox_node *tree)
{
    if (tree == NULL) return;
    if (TREE_NODATA_P (tree)) {
        CU_ASSERT ((tree->flags & LEAF) &&
                   (tree->flags & CONTAINS_HOLES) &&
                   (tree->leaf_data.dots_num == 0));
    } else {
        if (tree->flags & CONTAINS_HOLES) {
            CU_ASSERT (box_inside_box (&(tree->actual_bb), &(tree->data_bb), 0));
        } else {
            CU_ASSERT (vox_box_equalp (&(tree->actual_bb), &(tree->data_bb)));
        }
    }

    CU_ASSERT ((tree->flags & CONTAINS_HOLES) ||
               !(tree->flags & COVERED));

    int i;
    if (tree->flags & LEAF) {
        for (i=0; i<tree->leaf_data.dots_num; i++)
            CU_ASSERT (voxel_inside_box (&(tree->data_bb), tree->leaf_data.dots[i], 0));
    } else {
        CU_ASSERT (voxel_inside_box (&(tree->data_bb), tree->inner_data.center, 0));
        struct vox_box subspace;
        struct vox_node *child;
        for (i=0; i<8; i++) {
            subspace_box (&(tree->data_bb), tree->inner_data.center, &subspace, i);
            child = tree->inner_data.children[i];
            if (child != NULL)
                CU_ASSERT (box_inside_box (&subspace, &(child->actual_bb), 0));
            verify_structure (child);
        }
    }
}

static void test_verify_structure ()
{
    verify_structure (tree);
}

static void test_verify_content ()
{
    vox_dot voxel;
    int i,j,k, value, inside, p = 0;

    for (i=0; i<map->dim[0]; i++) {
        for (j=0; j<map->dim[1]; j++) {
            for (k=0; k<map->dim[2]; k++) {
                value = !!map->map[p];
                vox_dot_set (voxel, vox_voxel[0]*i, vox_voxel[1]*j, vox_voxel[2]*k);
                inside = vox_voxel_in_tree (tree, voxel);
                CU_ASSERT (!(inside ^ value));
                p++;
            }
        }
    }
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
    CU_pSuite voxtrees_ng = CU_add_suite ("voxtrees-ng", NULL, NULL);
    if (voxtrees_ng == NULL) PROC_SUIT_ERROR;

    test = CU_add_test (voxtrees_ng, "data loading", test_data_loading);
    if (test == NULL) PROC_TEST_ERROR;
    test = CU_add_test (voxtrees_ng, "tree construction", test_tree_cons);
    if (test == NULL) PROC_TEST_ERROR;
    test = CU_add_test (voxtrees_ng, "number of voxels", test_num_vox);
    if (test == NULL) PROC_TEST_ERROR;
    test = CU_add_test (voxtrees_ng, "verification of the tree structure", test_verify_structure);
    if (test == NULL) PROC_TEST_ERROR;
    test = CU_add_test (voxtrees_ng, "verification of the tree content", test_verify_content);
    if (test == NULL) PROC_TEST_ERROR;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
regcleanup: CU_cleanup_registry();
    if (map != NULL) vox_destroy_map_3d (map);
    vox_destroy_tree (tree);
exit_:    return res;
}
