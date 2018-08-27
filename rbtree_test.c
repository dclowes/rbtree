/* Merge of two versions:
 * Copyright (c) 2008 Derrick Coetzee
 * 2017 Douglas Clowes
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
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> /* rand() */
#include <string.h> /* strcmp() */

static int compare_int(const void* leftp, const void* rightp) {
    int left = * (int *)leftp;
    int right = * (int *)rightp;
    if (left < right)
        return -1;
    else if (left > right)
        return 1;
    else {
        assert (left == right);
        return 0;
    }
}

#define INDENT_STEP  4

static void print_tree_helper(rbtree_node n, int indent) {
    int i;
    if (n == NULL) {
        fputs("<empty tree>", stdout);
        return;
    }
    if (n->right != NULL) {
        print_tree_helper(n->right, indent + INDENT_STEP);
    }
    for(i=0; i<indent; i++)
        fputs(" ", stdout);
    if (n->color == BLACK)
        printf("%d\n", * (int *)n->key);
    else
        printf("<%d>\n", * (int *)n->key);
    if (n->left != NULL) {
        print_tree_helper(n->left, indent + INDENT_STEP);
    }
}

static void print_tree(rbtree t) {
    print_tree_helper(t->root, 0);
    puts("");
}

typedef struct {
    struct rbtree_node_t rbnode;
    int skey;
    int sval;
} data_node;

#ifndef MAXENT
#define MAXENT 50000
#endif

static int do_nuffin(rbtree_node node, void *context)
{
    (void) node;
    (void) context;
    return 0;
}

int main() {
    int inorder = 1;
    int invalid = 0;
    int lastkey;
    int i, keyz[MAXENT], valz[MAXENT];
    int min_key, max_key;
    int lo_key = 0, hi_key = 0;
    int num_dups = 0; /* number of duplicate keys */
    int num_miss = 0; /* number of missing deletes */
    int num_empty = 0; /* count of nodes on the tree after */
    int num_full = 0; /* count of nodes on the tree before */
    int num_up = 0; /* count going up */
    int num_dn = 0; /* count going down */
    struct rbtree_t tree;
    rbtree t = &tree;
    rbtree_node node;
    data_node *dnode;
    rbtree_init(t, (rbtree_compare_func) compare_int);
    if (rbtree_node_first(t))
        invalid = 1;
    if (rbtree_node_last(t))
        invalid = 1;
    if (rbtree_node_prev(t, NULL))
        invalid = 1;
    if (rbtree_node_next(t, NULL))
        invalid = 1;
    if (rbtree_walk(t, NULL, NULL))
        invalid = 1;
    if (rbtree_node_walk(NULL, NULL, NULL))
        invalid = 1;
    if (rbtree_node_delete(t, NULL))
        invalid = 1;
    print_tree(t);

    /*
     * Populate the tree
     */
    for(i=0; i<MAXENT; i++) {
        keyz[i] = rand() % (2 * MAXENT);
        valz[i] = rand() % (2 * MAXENT);
#ifdef TRACE
        int *x, *y;
        x = &keyz[i];
        y = &valz[i];
        print_tree(t);
        printf("Inserting %d -> %d\n\n", x, y);
#endif
        dnode = (data_node *) calloc(1, sizeof(data_node));
        node = &dnode->rbnode;
        node->key = &dnode->skey;
        node->value = &dnode->sval;
        *(int *)(node->key) = keyz[i];
        *(int *)(node->value) =valz[i];
        node = rbtree_insert(t, node);
        if (node != NULL) {
            ++num_dups;
            //printf("%4d Dup\n", x);
            free(node);
        }
        assert(rbtree_node_lookup(t, &keyz[i]));
        assert(*(int *)rbtree_lookup(t, &keyz[i]) == valz[i]);
    }

#ifdef TRACE
    print_tree(t);
#endif

    /*
     * Test the tree
     */
    min_key = keyz[0];
    max_key = keyz[0];
    for (i = 0; i < MAXENT; ++i) {
        if (min_key > keyz[i])
            min_key = keyz[i];
        if (max_key < keyz[i])
            max_key = keyz[i];
    }
    if ((node = rbtree_node_first(t))) {
        ++num_up;
        printf("Min=%d, First=%d\n", min_key, *(int *)node->key);
        lastkey = *(int *)node->key;
        hi_key = *(int *)node->key;
        while ((node = rbtree_node_next(t, node))) {
            ++num_up;
            if (lastkey > *(int *)node->key)
                inorder = 0;
            lastkey = *(int *)node->key;
#ifdef TRACE
            printf("Next = %d\n", *(int *)node->key);
#endif
            hi_key = *(int *)node->key;
        }
    }
    if ((node = rbtree_node_last(t))) {
        ++num_dn;
        printf("Max=%d, Last=%d\n", max_key, *(int *)node->key);
        lastkey = *(int *)node->key;
        lo_key = *(int *)node->key;
        while ((node = rbtree_node_prev(t, node))) {
            ++num_dn;
            if (lastkey < *(int *)node->key)
                inorder = 0;
            lastkey = *(int *)node->key;
#ifdef TRACE
            printf("Prev = %d\n", *(int *)node->key);
#endif
            lo_key = *(int *)node->key;
        }
    }
    printf("Head=%d, Tail=%d\n", lo_key, hi_key);

    /*
     * Depopulate the tree
     */
    num_full = rbtree_walk(t, do_nuffin, NULL);
    for(i=0; i<MAXENT; i++) {
        int *x = &keyz[i];
#ifdef TRACE
        print_tree(t);
        printf("Deleting key %d\n\n", x);
#endif
        rbtree_node n = rbtree_delete(t, (void*)x);
        if(n != NULL) {
            free(n);
            if (15 == t->node_count)
                print_tree(t);
        } else {
            ++num_miss;
            //printf("%4d xxx\n", x);
        }
    }
    print_tree(t);
    num_empty = rbtree_walk(t, NULL, NULL);
    printf("MAXENT=%d, inorder=%s, num_dups=%d, num_miss=%d\n",
           MAXENT, inorder ? "true" : "false", num_dups, num_miss);
    printf("num_full=%d, num_empty=%d",
           num_full, num_empty);
    printf(", num_up=%d, num_dn=%d\n",
           num_up, num_dn);

    /* Check and count errors */
    int errors = 0;
    if (MAXENT != (num_dups + num_full))
        printf("%2d: failed MAXENT != (num_dups + num_full))\n", ++errors);
    if (min_key != lo_key)
        printf("%2d: failed min_key != lo_key\n", ++errors);
    if (max_key != hi_key)
        printf("%2d: failed max_key != hi_key\n", ++errors);
    if (num_up != num_full)
        printf("%2d: failed num_up != num_full\n", ++errors);
    if (num_dn != num_full)
        printf("%2d: failed num_dn != num_full\n", ++errors);
    if (num_dups != num_miss)
        printf("%2d: failed num_dups != num_miss\n", ++errors);
    if (num_full == 0)
        printf("%2d: failed num_full == 0\n", ++errors);
    if (num_empty != 0)
        printf("%2d: failed num_empty != 0\n", ++errors);
    if (invalid == 1)
        printf("%2d: failed invalid == 1)\n", ++errors);
    if (inorder == 0)
        printf("%2d: failed inorder == 0)\n", ++errors);
    if (errors) {
        printf("Failed\n");
        return EXIT_FAILURE;
    } else {
        printf("Okay\n");
        return EXIT_SUCCESS;
    }
}
/* vim: set ts=8 sw=4 sts=4: */
