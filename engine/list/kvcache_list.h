#ifndef __KVCACHE_LIST_H__
#define __KVCACHE_LIST_H__

#include "list.h"
#include "kvcache_ops.h"

typedef struct
{
    elem_t *_key;
    elem_t *_val;
    struct list_head node;
}list_elem_t;

typedef struct
{   
    kvcache_ops_t _ops;
    struct list_head head;
}kvcache_list_ops_t;

kvcache_ops_t* create_kvcache_list_ops(void);
int destroy_kvcache_list_ops(kvcache_ops_t* ops);

#endif // !__KVCACHE_LIST_H__