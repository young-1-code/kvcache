#ifndef __KVCACHE_HASHTABLE_H__
#define __KVCACHE_HASHTABLE_H__

#include "kvcache_ops.h"


#define HASH_TABLE_BUCKET_CNT_LEVEL 28 
#define INC_LOAD_FACTOR_MAX 1.0
#define DEC_LOAD_FACTOR_MIN 0.5

static const uint32_t bucket_cnt_array[HASH_TABLE_BUCKET_CNT_LEVEL] = {
    53ul,         97ul,         193ul,      389ul,       769ul,
    1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
    1610612741ul, 3221225473ul, 4294967291ul
};


typedef size_t (*hash_key)(const elem_t *_key);

struct hnode_t
{
    struct hnode_t* next;
    elem_t *_key; /* key */
    elem_t *_val; /* val */
};

struct htable_t /* hash表 */
{
    struct hnode_t  **tb;         /* hash表              */
    hash_key        hash_fn;      /* 计算hash值          */
    uint32_t        bucket_n;     /* 当前最大桶个数       */
    uint32_t        node_n;       /* 当前节点数           */
    int             bucket_index; /* 桶值数组下标         */
    float           factor;       /* factor负载因子       */
};

typedef struct
{   
    kvcache_ops_t   _ops;
    struct htable_t *htable[2]; /* 0:正常使用的 1:rehash时候使用 */
}kvcache_htable_ops_t;

kvcache_ops_t* create_kvcache_hashtable_ops(void);
int destroy_kvcache_hashtable_ops(kvcache_ops_t* ops);

#endif // !__KVCACHE_HASHTABLE_H__

