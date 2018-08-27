#!/bin/bash
gcc -g -pg --coverage rbtree_test.c rbtree.c -o a.out && \
  ./a.out && \
  gcov rbtree.c && \
  gcov rbtree_test.c && \
  gprof > rbtree_test.gprof
