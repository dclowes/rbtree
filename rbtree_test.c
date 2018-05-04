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

static int compare_int(const void* left, const void* right);
static void print_tree(rbtree t);
static void print_tree_helper(rbtree_node n, int indent);

int compare_int(const void* leftp, const void* rightp) {
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

void print_tree_helper(rbtree_node n, int indent);

void print_tree(rbtree t) {
    print_tree_helper(t->root, 0);
    puts("");
}

void print_tree_helper(rbtree_node n, int indent) {
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

typedef struct {
    struct rbtree_node_t rbnode;
    int skey;
    int sval;
} data_node;

#ifndef MAXENT
#define MAXENT 5000
#endif

int main() {
    int inorder = 1;
    int lastkey;
    int i, keyz[MAXENT], valz[MAXENT];
    int min_key, max_key;
    int lo_key, hi_key;
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
    print_tree(t);

    /*
     * Populate the tree
     */
    for(i=0; i<MAXENT; i++) {
        int *x, *y;
        keyz[i] = rand() % (2 * MAXENT);
        valz[i] = rand() % (2 * MAXENT);
        x = &keyz[i];
        y = &valz[i];
#ifdef TRACE
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
        while ((node = rbtree_node_next(t, node))) {
            ++num_up;
            if (lastkey > *(int *)node->key)
                inorder = 0;
            lastkey > *(int *)node->key;
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
        while ((node = rbtree_node_prev(t, node))) {
            ++num_dn;
            if (lastkey < *(int *)node->key)
                inorder = 0;
            lastkey > *(int *)node->key;
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
    num_full = rbtree_walk(t, NULL, NULL);
    for(i=0; i<MAXENT; i++) {
        int *x = &keyz[i];
#ifdef TRACE
        print_tree(t);
        printf("Deleting key %d\n\n", x);
#endif
        rbtree_node n = rbtree_delete(t, (void*)x);
        if(n != NULL) {
            free(n);
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
    if (MAXENT != (num_dups + num_full)
        || min_key != lo_key
        || max_key != hi_key
        || num_up != num_full
        || num_dn != num_full
        || num_dups != num_miss
        || num_full == 0
        || num_empty != 0)
        printf("Failed\n");
    else
        printf("Okay\n");
    return 0;
}
/* vim: set ts=8 sw=4 sts=4: */
