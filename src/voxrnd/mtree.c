#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mtree.h"
#include "../voxtrees/geom.h"

#define sphere_copy(dst,src) memcpy (dst, src, sizeof (struct vox_sphere))

static struct vox_mtree_node* alloc_node (int leaf)
{
    struct vox_mtree_node *node = vox_alloc (sizeof (struct vox_mtree_node));
    memset (node, 0, sizeof (struct vox_mtree_node));
    node->leaf = leaf;
    if (leaf) node->data.spheres = vox_alloc (sizeof (struct vox_sphere) * (MTREE_MAX_CHILDREN + 1));

    return node;
}

// Returns 1 if the sphere was changed
static int bounding_sphere (const struct vox_sphere *s, struct vox_sphere *bounding_sphere)
{
    float dist, crit;
    int changed = 1;

    /*
     * There is school-level math behind this. We try to optimize the
     * bounding sphere by minimizing its radius. Turns out, a center
     * of the new bounding sphere must be between centers of two
     * spheres (kinda obvious) and radius must be calculated as
     * follows.
     */
    dist = sqrtf (vox_sqr_metric (s->center, bounding_sphere->center));
    crit = (1 + (bounding_sphere->radius - s->radius) / dist) / 2;

    if (crit < 0) {
        sphere_copy (bounding_sphere, s);
    } else if (crit < 1) {
        bounding_sphere->radius = (s->radius + bounding_sphere->radius + dist) / 2;
        bounding_sphere->sqr_radius = pow (bounding_sphere->radius, 2);
        vox_dot_sub (bounding_sphere->center, s->center, bounding_sphere->center);
        vox_dot_scmul (bounding_sphere->center, crit, bounding_sphere->center);
        vox_dot_add (bounding_sphere->center, s->center, bounding_sphere->center);
    } else changed = 0;

    return changed;
}

// Calculate bounding sphere for a node
static int node_bounding_sphere (struct vox_mtree_node *node)
{
    assert (node != NULL && node->num > 0);
    unsigned int i;
    int changed = 0;

    sphere_copy (&(node->bounding_sphere),
                 (node->leaf)? &(node->data.spheres[0]):
                 &(node->data.children[0]->bounding_sphere));

    for (i=1; i<node->num; i++) {
        changed += bounding_sphere ((node->leaf)? &(node->data.spheres[i]):
                                    &(node->data.children[i]->bounding_sphere),
                                    &(node->bounding_sphere));
    }
    return !!changed;
}

/*
 * Propagate bounding sphere recalculation to the parent (of the
 * parent of the parent...) Stop if the new bounding sphere matches
 * the old.
 */
static void propagate_bounding_sphere_update (struct vox_mtree_node *node)
{
    if (node != NULL) {
        int changed = node_bounding_sphere (node);
        if (changed) propagate_bounding_sphere_update (node->parent);
    }
}

// Find farest items to do a split.
static void farest_centers (const struct vox_mtree_node *node, vox_dot c1, vox_dot c2)
{
    size_t i,j;
    float dist, maxdist = 0;
    assert (node != NULL && node->num > 1);

    vox_dot dot1, dot2;

    for (i=0; i<node->num; i++) {
        for (j=i+1; j<node->num; j++) {
            vox_dot_copy (dot1, (node->leaf)? node->data.spheres[i].center:
                          node->data.children[i]->bounding_sphere.center);
            vox_dot_copy (dot2, (node->leaf)? node->data.spheres[j].center:
                          node->data.children[j]->bounding_sphere.center);

            dist = vox_sqr_metric (dot1, dot2);
            if (dist > maxdist) {
                vox_dot_copy (c1, dot1);
                vox_dot_copy (c2, dot2);
                maxdist = dist;
            }
        }
    }
}

// Returns last node not affected by split (with exception of bs change) or a new root
static struct vox_mtree_node* split_node (struct vox_mtree_node *node)
{
    // Ensure that we must split
    assert (node->num == MTREE_MAX_CHILDREN+1);

    vox_dot center1, center2;
    float dist1, dist2;
    unsigned int i;

    struct vox_mtree_node *which, *res, *parent;
    parent = node->parent;
    res = parent;

    struct vox_mtree_node *n1 = alloc_node (node->leaf);
    struct vox_mtree_node *n2 = alloc_node (node->leaf);

    n1->parent = parent;
    n2->parent = parent;

    farest_centers (node, center1, center2);
    for (i=0; i<node->num; i++) {
        dist1 = vox_sqr_metric ((node->leaf)? node->data.spheres[i].center:
                                node->data.children[i]->bounding_sphere.center, center1);
        dist2 = vox_sqr_metric ((node->leaf)? node->data.spheres[i].center:
                                node->data.children[i]->bounding_sphere.center, center2);

        which = (dist1 < dist2)? n1: n2;
        if (node->leaf) sphere_copy (&(which->data.spheres[which->num]), &(node->data.spheres[i]));
        else {
            which->data.children[which->num] = node->data.children[i];
            node->data.children[i]->parent = which;
        }
        which->num++;
    }

    // Update bs
    node_bounding_sphere (n1);
    node_bounding_sphere (n2);

    // Free splitted node
    memset (node, 0, sizeof (struct vox_mtree_node));
    free (node);

    // Update parent
    if (parent != NULL) {
        assert ((!parent->leaf) && parent->num <= MTREE_MAX_CHILDREN);
        for (i=0; i<parent->num; i++)
            if (parent->data.children[i] == node) parent->data.children[i] = n1;
        parent->data.children[parent->num] = n2;
        parent->num++;

        // Propagate changes
        propagate_bounding_sphere_update (parent); // Do we really need this? See vox_mtree_add_spheres()
        if (parent->num == MTREE_MAX_CHILDREN + 1) res = split_node (parent);
    } else {
        // Create a new root
        res = alloc_node (0);
        res->num = 2;
        res->data.children[0] = n1;
        res->data.children[1] = n2;
        node_bounding_sphere (res);
        n1->parent = res;
        n2->parent = res;
    }

    return res;
}

const struct vox_mtree_node*
vox_mtree_contains_sphere (const struct vox_mtree_node *node,
                           const struct vox_sphere *s)
{
    const struct vox_mtree_node* res = NULL;
    unsigned int i;
    float dist;
    const struct vox_mtree_node *child;

    if (node != NULL) {
        dist = vox_sqr_metric (s->center, node->bounding_sphere.center);
        if (dist < node->bounding_sphere.sqr_radius) {
            if (node->leaf) {
                for (i=0; i<node->num; i++) {
                    if (vox_dot_equalp (node->data.spheres[i].center, s->center) &&
                        node->data.spheres[i].radius == s->radius) {
                        res = node;
                        break;
                    }
                }
            } else {
                assert (node->num > 0);
                for (i=0; i<node->num; i++) {
                    child = node->data.children[i];
                    res = vox_mtree_contains_sphere (child, s);
                    if (res != NULL) break;
                }
            }
        }
    }

    return res;
}

static struct vox_mtree_node* add_sphere_helper (struct vox_mtree_node *node,
                                                 const struct vox_sphere *s)
{
    struct vox_mtree_node *res = node;
    struct vox_mtree_node *child;
    unsigned int i;
    float sqr_radius = pow (s->radius, 2);
    float dist, mindist = INFINITY;

    if (node == NULL) {
        res = alloc_node (1);
        sphere_copy (&(res->bounding_sphere), s);
        res->bounding_sphere.sqr_radius = sqr_radius;
        res->num = 1;
        sphere_copy (&(res->data.spheres[0]), s);
        res->data.spheres[0].sqr_radius = sqr_radius;
    } else if (node->leaf) {
        sphere_copy (&(node->data.spheres[node->num]), s);
        node->data.spheres[node->num].sqr_radius = sqr_radius;
        node->num++;
        propagate_bounding_sphere_update (node);
        if (node->num == MTREE_MAX_CHILDREN + 1)
            res = split_node (node);
    } else {
        assert (node->num > 0);
        for (i=0; i<node->num; i++) {
            dist = vox_sqr_metric (s->center, node->data.children[i]->bounding_sphere.center);
            if (dist < mindist) {
                mindist = dist;
                child = node->data.children[i];
            }
        }
        res = add_sphere_helper (child, s);
    }

    return res;
}

int vox_mtree_add_sphere (struct vox_mtree_node **nodeptr, const struct vox_sphere *s)
{
    // Sanity check and make sure that the sphere is not already in the tree
    int success = (s->radius > 0) && (vox_mtree_contains_sphere (*nodeptr, s) == NULL);

    if (success) {
        struct vox_mtree_node *node = add_sphere_helper (*nodeptr, s);
        // Got a new root
        if (node->parent == NULL) *nodeptr = node;
    }

    return success;
}

// This is "split_node() for deletion"
static struct vox_mtree_node* recursively_delete (struct vox_mtree_node *node)
{
    assert (node->num == 1);
    struct vox_mtree_node *parent = node->parent;
    struct vox_mtree_node *res = node;
    unsigned int i;

    vox_mtree_destroy (node);
    if (parent == NULL) return NULL;

    assert (!parent->leaf && parent->num > 0);

    if (parent->num == 1) {
        parent->data.children[0] = NULL;
        return recursively_delete (parent);
    }

    for (i=0; i<parent->num; i++) {
        if (parent->data.children[i] == node) break;
    }
    assert (i < parent->num);

    memcpy (&(parent->data.children[i]), &(parent->data.children[i+1]),
            sizeof (struct vox_mtree_node*) * (parent->num - i - 1));
    parent->num--;
    parent->data.children[parent->num] = NULL;
    propagate_bounding_sphere_update (parent);

    return res;
}

static int remove_sphere_helper (struct vox_mtree_node *node, const struct vox_sphere *s)
{
    unsigned int i;
    int tree_empty = 0;

    assert (node != NULL && node->leaf);
    if (node->num == 1) {
        node = recursively_delete (node);
        if (node == NULL) tree_empty = 1;
    } else {
        for (i=0; i<node->num; i++) {
            if (vox_dot_equalp (node->data.spheres[i].center, s->center) &&
                node->data.spheres[i].radius == s->radius) break;
        }

        assert (i < node->num);
        memcpy (&(node->data.spheres[i]), &(node->data.spheres[i+1]),
                sizeof (struct vox_sphere) * (node->num - i - 1));
        node->num--;
        propagate_bounding_sphere_update (node);
    }

    return tree_empty;
}

int vox_mtree_remove_sphere (struct vox_mtree_node **nodeptr, const struct vox_sphere *s)
{
    struct vox_mtree_node *leaf = vox_mtree_contains_sphere (*nodeptr, s);
    int success = (s->radius > 0) && (leaf != NULL);

    if (success) {
        if (remove_sphere_helper (leaf, s)) {
            // Tree is empty
            *nodeptr = NULL;
        }
    }

    return success;
}

void vox_mtree_destroy (struct vox_mtree_node *node)
{
    unsigned int i;

    if (node != NULL) {
        if (node->leaf) free (node->data.spheres);
        else {
            for (i=0; i<node->num; i++) vox_mtree_destroy (node->data.children[i]);
        }
        free (node);
    }
}

void vox_mtree_dump (const struct vox_mtree_node *node)
{
    unsigned int i;

    printf ("Node %p ", node);
    if (node == NULL) {
        printf ("empty node\n");
        return;
    }

    printf ("%s, parent=%p\n", (node->leaf)? "leaf": "inner node", node->parent);
    printf ("Bounding sphere: <%7.4f, %7.4f, %7.4f>, %7.4f\n",
            node->bounding_sphere.center[0],
            node->bounding_sphere.center[1],
            node->bounding_sphere.center[2],
            node->bounding_sphere.radius);

    if (node->leaf) {
        printf ("Leaf content (%u spheres):\n", node->num);
        for (i=0; i<node->num; i++) {
            printf ("<%7.4f, %7.4f, %7.4f>, %7.4f\n",
                    node->data.spheres[i].center[0],
                    node->data.spheres[i].center[1],
                    node->data.spheres[i].center[2],
                    node->data.spheres[i].radius);
        }
        printf ("==========\n");
    } else {
        printf ("Node children\n");
        for (i=0; i<node->num; i++) printf ("%p ", node->data.children[i]);
        printf ("\n==========\n");
        for (i=0; i<node->num; i++) vox_mtree_dump (node->data.children[i]);
    }
}

unsigned int vox_mtree_items (const struct vox_mtree_node *node)
{
    unsigned int i, count = 0;
    if (node != NULL) {
        if (node->leaf) count = node->num;
        else {
            for (i=0; i<node->num; i++) count += vox_mtree_items (node->data.children[i]);
        }
    }

    return count;
}

void vox_mtree_spheres_containing (const struct vox_mtree_node *node, const vox_dot dot,
                                   void (^block)(const struct vox_sphere *s))
{
    float dist;
    unsigned int i;

    if (node != NULL)
    {
        dist = vox_sqr_metric (node->bounding_sphere.center, dot);
        if (dist < node->bounding_sphere.sqr_radius) {
            if (node->leaf) {
                if (node->num == 1) {
                    /*
                     * We already found our sphere, no need to call
                     * vox_sqr_metric() the second time.
                     */
                    block (&(node->data.spheres[0]));
                    return;
                }
                for (i=0; i<node->num; i++) {
                    dist = vox_sqr_metric (node->data.spheres[i].center, dot);
                    if (dist < node->data.spheres[i].sqr_radius) block (&(node->data.spheres[i]));
                }
            } else {
                for (i=0; i<node->num; i++)
                    vox_mtree_spheres_containing (node->data.children[i], dot, block);
            }
        }
    }
}
