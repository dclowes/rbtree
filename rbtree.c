/* Merge of two versions:
 * Copyright (c) 2008 Derrick Coetzee
 * Douglas Clowes, February 2017
 *
 * Retrieved from:
 * http://en.literateprograms.org/Red-black_tree_(C)?oldid=7982
 * http://en.literateprograms.org/Red-black_tree_(C)?oldid=19567
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "rbtree.h"
#include <assert.h>
#include <stdlib.h>

typedef rbtree_node node;
typedef enum rbtree_node_color color;

static node grandparent(node n);
static node sibling(node n);
static node uncle(node n);
static color node_color(node n);

#ifdef VERIFY_RBTREE
static void verify_properties(rbtree t);
static void verify_property_1(node root);
static void verify_property_2(node root);
static void verify_property_4(node root);
static void verify_property_5(node root);
static void verify_property_5_helper(node n, int black_count, int* black_count_path);
#else
/* Make it go away */
#define verify_properties(node)
#endif

static node lookup_node(rbtree t, const void* key);
static void rotate_left(rbtree t, node n);
static void rotate_right(rbtree t, node n);

static void replace_node(rbtree t, node oldn, node newn);
static void insert_case1(rbtree t, node n);
static void insert_case2(rbtree t, node n);
static void insert_case3(rbtree t, node n);
static void insert_case4(rbtree t, node n);
static void insert_case5(rbtree t, node n);
static node maximum_node(node root);
static void delete_case1(rbtree t, node n);
static void delete_case2(rbtree t, node n);
static void delete_case3(rbtree t, node n);
static void delete_case4(rbtree t, node n);
static void delete_case5(rbtree t, node n);
static void delete_case6(rbtree t, node n);

static node grandparent(node n) {
    assert (n != NULL);
    assert (n->parent != NULL); /* Not the root node */
    assert (n->parent->parent != NULL); /* Not child of root */
    return n->parent->parent;
}

static node sibling(node n) {
    assert (n != NULL);
    assert (n->parent != NULL); /* Root node has no sibling */
    if (n == n->parent->left)
        return n->parent->right;
    else
        return n->parent->left;
}

static node uncle(node n) {
    assert (n != NULL);
    assert (n->parent != NULL); /* Root node has no uncle */
    assert (n->parent->parent != NULL); /* Children of root have no uncle */
    return sibling(n->parent);
}

static color node_color(node n) {
    return n == NULL ? BLACK : n->color;
}

#ifdef VERIFY_RBTREE
static void verify_properties(rbtree t) {
    verify_property_1(t->root);
    verify_property_2(t->root);
    /* Property 3 is implicit */
    verify_property_4(t->root);
    verify_property_5(t->root);
}

static void verify_property_1(node n) {
    assert(node_color(n) == RED || node_color(n) == BLACK);
    if (n == NULL) return;
    verify_property_1(n->left);
    verify_property_1(n->right);
}

static void verify_property_2(node root) {
    assert(node_color(root) == BLACK);
}

static void verify_property_4(node n) {
    if (node_color(n) == RED) {
        assert (node_color(n->left)   == BLACK);
        assert (node_color(n->right)  == BLACK);
        assert (node_color(n->parent) == BLACK);
    }
    if (n == NULL) return;
    verify_property_4(n->left);
    verify_property_4(n->right);
}

static void verify_property_5(node root) {
    int black_count_path = -1;
    verify_property_5_helper(root, 0, &black_count_path);
}

static void verify_property_5_helper(node n, int black_count, int* path_black_count) {
    if (node_color(n) == BLACK) {
        black_count++;
    }
    if (n == NULL) {
        if (*path_black_count == -1) {
            *path_black_count = black_count;
        } else {
            assert (black_count == *path_black_count);
        }
        return;
    }
    verify_property_5_helper(n->left,  black_count, path_black_count);
    verify_property_5_helper(n->right, black_count, path_black_count);
}
#endif

void rbtree_init(rbtree t, rbtree_compare_func compare) {
    t->root = NULL;
    t->compare = compare;
    t->node_count = 0;

    verify_properties(t);
}

static node lookup_node(rbtree t, const void* key) {
    node n = t->root;
    while (n != NULL) {
        int comp_result = t->compare(key, n->key);
        if (comp_result == 0) {
            return n;
        } else if (comp_result < 0) {
            n = n->left;
        } else {
            assert(comp_result > 0);
            n = n->right;
        }
    }
    return n;
}

void* rbtree_lookup(rbtree t, const void* key) {
    node n = lookup_node(t, key);
    return n == NULL ? NULL : n->value;
}

static void rotate_left(rbtree t, node n) {
    node r = n->right;
    replace_node(t, n, r);
    n->right = r->left;
    if (r->left != NULL) {
        r->left->parent = n;
    }
    r->left = n;
    n->parent = r;
}

static void rotate_right(rbtree t, node n) {
    node L = n->left;
    replace_node(t, n, L);
    n->left = L->right;
    if (L->right != NULL) {
        L->right->parent = n;
    }
    L->right = n;
    n->parent = L;
}

static void replace_node(rbtree t, node oldn, node newn) {
    if (oldn->parent == NULL) {
        t->root = newn;
    } else {
        if (oldn == oldn->parent->left)
            oldn->parent->left = newn;
        else
            oldn->parent->right = newn;
    }
    if (newn != NULL) {
        newn->parent = oldn->parent;
    }
}

rbtree_node rbtree_insert(rbtree t, rbtree_node inserted_node) {
    inserted_node->color = RED;
    inserted_node->left = NULL;
    inserted_node->right = NULL;
    inserted_node->parent = NULL;

    if (t->root == NULL) {
        t->root = inserted_node;
    } else {
        node n = t->root;
        while (1) {
            int comp_result = t->compare(inserted_node->key, n->key);
            if (comp_result == 0) {
                /* key exists: swap nodes */
                inserted_node->left = n->left;
                inserted_node->right = n->right;
                inserted_node->parent = n->parent;
                inserted_node->color = n->color;
                replace_node(t, n, inserted_node);
                if (inserted_node->left)
                    inserted_node->left->parent = inserted_node;
                if (inserted_node->right)
                    inserted_node->right->parent = inserted_node;
                /* return replaced node for disposal */
                return n;
            } else if (comp_result < 0) {
                if (n->left == NULL) {
                    n->left = inserted_node;
                    break;
                } else {
                    n = n->left;
                }
            } else {
                assert (comp_result > 0);
                if (n->right == NULL) {
                    n->right = inserted_node;
                    break;
                } else {
                    n = n->right;
                }
            }
        }
        inserted_node->parent = n;
    }
    insert_case1(t, inserted_node);

    t->node_count += 1;
    verify_properties(t);
    return NULL;
}

static void insert_case1(rbtree t, node n) {
    if (n->parent == NULL)
        n->color = BLACK;
    else
        insert_case2(t, n);
}

static void insert_case2(rbtree t, node n) {
    if (node_color(n->parent) == BLACK)
        return; /* Tree is still valid */
    else
        insert_case3(t, n);
}

static void insert_case3(rbtree t, node n) {
    if (node_color(uncle(n)) == RED) {
        n->parent->color = BLACK;
        uncle(n)->color = BLACK;
        grandparent(n)->color = RED;
        insert_case1(t, grandparent(n));
    } else {
        insert_case4(t, n);
    }
}

static void insert_case4(rbtree t, node n) {
    if (n == n->parent->right && n->parent == grandparent(n)->left) {
        rotate_left(t, n->parent);
        n = n->left;
    } else if (n == n->parent->left && n->parent == grandparent(n)->right) {
        rotate_right(t, n->parent);
        n = n->right;
    }
    insert_case5(t, n);
}

static void insert_case5(rbtree t, node n) {
    n->parent->color = BLACK;
    grandparent(n)->color = RED;
    if (n == n->parent->left && n->parent == grandparent(n)->left) {
        rotate_right(t, grandparent(n));
    } else {
        assert (n == n->parent->right && n->parent == grandparent(n)->right);
        rotate_left(t, grandparent(n));
    }
}

rbtree_node rbtree_delete(rbtree t, const void* key) {
    node n = lookup_node(t, key);
    if (n == NULL) return NULL;  /* Key not found, do nothing */
    return rbtree_node_delete(t, n);
}

static node minimum_node(node n) {
    assert (n != NULL);
    while (n->left != NULL) {
        n = n->left;
    }
    return n;
}

static node maximum_node(node n) {
    assert (n != NULL);
    while (n->right != NULL) {
        n = n->right;
    }
    return n;
}

static void delete_case1(rbtree t, node n) {
    if (n->parent == NULL)
        return;
    else
        delete_case2(t, n);
}

static void delete_case2(rbtree t, node n) {
    if (node_color(sibling(n)) == RED) {
        n->parent->color = RED;
        sibling(n)->color = BLACK;
        if (n == n->parent->left)
            rotate_left(t, n->parent);
        else
            rotate_right(t, n->parent);
    }
    delete_case3(t, n);
}

static void delete_case3(rbtree t, node n) {
    if (node_color(n->parent) == BLACK &&
        node_color(sibling(n)) == BLACK &&
        node_color(sibling(n)->left) == BLACK &&
        node_color(sibling(n)->right) == BLACK)
    {
        sibling(n)->color = RED;
        delete_case1(t, n->parent);
    }
    else
        delete_case4(t, n);
}

static void delete_case4(rbtree t, node n) {
    if (node_color(n->parent) == RED &&
        node_color(sibling(n)) == BLACK &&
        node_color(sibling(n)->left) == BLACK &&
        node_color(sibling(n)->right) == BLACK)
    {
        sibling(n)->color = RED;
        n->parent->color = BLACK;
    }
    else
        delete_case5(t, n);
}

static void delete_case5(rbtree t, node n) {
    if (n == n->parent->left &&
        node_color(sibling(n)) == BLACK &&
        node_color(sibling(n)->left) == RED &&
        node_color(sibling(n)->right) == BLACK)
    {
        sibling(n)->color = RED;
        sibling(n)->left->color = BLACK;
        rotate_right(t, sibling(n));
    }
    else if (n == n->parent->right &&
             node_color(sibling(n)) == BLACK &&
             node_color(sibling(n)->right) == RED &&
             node_color(sibling(n)->left) == BLACK)
    {
        sibling(n)->color = RED;
        sibling(n)->right->color = BLACK;
        rotate_left(t, sibling(n));
    }
    delete_case6(t, n);
}

static void delete_case6(rbtree t, node n) {
    sibling(n)->color = node_color(n->parent);
    n->parent->color = BLACK;
    if (n == n->parent->left) {
        assert (node_color(sibling(n)->right) == RED);
        sibling(n)->right->color = BLACK;
        rotate_left(t, n->parent);
    }
    else
    {
        assert (node_color(sibling(n)->left) == RED);
        sibling(n)->left->color = BLACK;
        rotate_right(t, n->parent);
    }
}

/*
 * Additional methods
 */
rbtree_node rbtree_node_lookup(rbtree t, const void* key)
{
    return lookup_node(t, key);
}

rbtree_node rbtree_node_delete(rbtree t, rbtree_node n)
{
    node child;
    if (n == NULL)
        return NULL;
    if (n->left != NULL && n->right != NULL) {
        /* node has two children: swap position with predecessor */
        struct rbtree_node_t *temp;
        enum rbtree_node_color color;
        node pred = maximum_node(n->left);
        n->left->parent = pred;
        if (pred->left)
            pred->left->parent = n;
        n->right->parent = pred;
        if (pred->right)
            pred->right->parent = n;
        temp = pred->left;
        pred->left = n->left;
        n->left = temp;
        temp = pred->right;
        pred->right = n->right;
        n->right = temp;
        temp = pred->parent;
        pred->parent = n->parent;
        n->parent = temp;
        color = pred->color;
        pred->color = n->color;
        n->color = color;
        if (pred->parent == NULL)
            t->root = pred;
        else {
            if (pred->parent->left == n)
                pred->parent->left = pred;
            else
                pred->parent->right = pred;
        }
        if (n->parent->left == pred)
            n->parent->left = n;
        else
            n->parent->right = n;
    }

    assert(n->left == NULL || n->right == NULL);
    child = n->right == NULL ? n->left  : n->right;
    if (node_color(n) == BLACK) {
        n->color = node_color(child);
        delete_case1(t, n);
    }
    replace_node(t, n, child);
    /* TODO check next two lines, should be removed? */
    if (n->parent == NULL && child != NULL) // root should be black
        child->color = BLACK;

    t->node_count -= 1;
    verify_properties(t);
    return n;
}

rbtree_node rbtree_node_first(rbtree t)
{
    if (t == NULL || t->root == NULL)
        return NULL;
    return minimum_node(t->root);
}

rbtree_node rbtree_node_last(rbtree t)
{
    if (t == NULL || t->root == NULL)
        return NULL;
    return maximum_node(t->root);
}

rbtree_node rbtree_node_prev(rbtree t, rbtree_node node)
{
    rbtree_node parent;

    (void)t;

    if (node == NULL)
        return NULL;

    /*
     * If we have a left-hand child, return max(left)
     */
    if (node->left)
        return maximum_node(node->left);

    /*
     * No left-hand children, go up until we find
     * an ancestor that is the right-hand child of its parent
     */
    while ((parent = node->parent) && node != parent->right)
        node = parent;

    return parent;
}

rbtree_node rbtree_node_next(rbtree t, rbtree_node node)
{
    rbtree_node parent;

    (void)t;

    if (node == NULL)
        return NULL;

    /*
     * If we have a right-hand child, return min(right)
     */
    if (node->right)
        return minimum_node(node->right);

    /*
     * No right-hand children, go up until we find
     * an ancestor that is the left-hand child of its parent
     */
    while ((parent = node->parent) && node != parent->left)
        node = parent;

    return parent;
}

static int node_walk(rbtree_node node, rbtree_visitor_func f, void *context)
{
    int count = 0;

    if (node == NULL)
        return 0;
    if (node->left)
        count += node_walk(node->left, f, context);
    count++;
    if (f)
        f(node, context);
    if (node->right)
        count += node_walk(node->right, f, context);
    return count;
}

int rbtree_node_walk(rbtree_node node, rbtree_visitor_func f, void *context)
{
    return node_walk(node, f, context);
}

int rbtree_walk(rbtree t, rbtree_visitor_func f, void *context)
{
    if (t && t->root)
        return node_walk(t->root, f, context);
    return 0;
}

/* vim: set ts=8 sw=4 sts=4 et: */
