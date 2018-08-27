/* Merge of two versions:
 * Copyright (c) 2008 Derrick Coetzee
 * Small changes by Ryah Dahl
 * Douglas Clowes, February 2017
 *
 * Retrieved from:
 * http://en.literateprograms.org/Red-black_tree_(C)?oldid=7982
 * http://en.literateprograms.org/Red-black_tree_(C)?oldid=19567
 * https://github.com/taf2/libebb
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

#ifndef _RBTREE_H_
#define _RBTREE_H_

enum rbtree_node_color { RED, BLACK };

typedef int (*rbtree_compare_func)(const void* left_key, const void* right_key);

typedef struct rbtree_node_t {
    void* key;
    void* value;
    struct rbtree_node_t* left;
    struct rbtree_node_t* right;
    struct rbtree_node_t* parent;
    enum rbtree_node_color color;
} *rbtree_node;

typedef struct rbtree_t {
    rbtree_node root;
    rbtree_compare_func compare;  /* private */
    int node_count;
} *rbtree;

typedef int (*rbtree_visitor_func)(rbtree_node node, void* context);

void rbtree_init(rbtree t, rbtree_compare_func);
void* rbtree_lookup(rbtree t, const void* key);
/* you must free the returned node */
rbtree_node rbtree_insert(rbtree t, rbtree_node);
/* you must free the returned node */
rbtree_node rbtree_delete(rbtree t, const void* key);

/*
 * Additional methods
 */
rbtree_node rbtree_node_lookup(rbtree t, const void* key);
rbtree_node rbtree_node_delete(rbtree t, rbtree_node node);
rbtree_node rbtree_node_first(rbtree t);
rbtree_node rbtree_node_last(rbtree t);
rbtree_node rbtree_node_prev(rbtree t, rbtree_node node);
rbtree_node rbtree_node_next(rbtree t, rbtree_node node);
int rbtree_node_walk(rbtree_node node, rbtree_visitor_func f, void *context);
int rbtree_walk(rbtree t, rbtree_visitor_func f, void *context);
/* vim: set ts=8 sw=4 sts=4: */
#endif
