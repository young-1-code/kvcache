#ifndef __RBTREE_H__
#define __RBTREE_H__

#include "rbtree.h"
#include "kvcache_ops.h"

typedef struct
{
    struct rb_node node;
    elem_t *_key;
    elem_t *_val;
}rbtree_elem_t;

typedef struct
{   
    kvcache_ops_t _ops;
    struct rb_root root;
    int node_cnt;
}kvcache_rbtree_ops_t;

kvcache_ops_t* create_kvcache_rbtree_ops(void);
int destroy_kvcache_rbtree_ops(kvcache_ops_t* ops);


#endif