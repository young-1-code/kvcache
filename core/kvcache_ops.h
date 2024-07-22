#ifndef __KVCACHE_CMD_H__
#define __KVCACHE_CMD_H__

#include <stdint.h>

static const char *kv_class_name[] = 
{
    "list",
    "hashtable",
    "skiptable",
    "rbtree",
    "btree"
};

typedef enum 
{
    KVCACHE_OPS_START=0,
    KVCACHE_OPS_LIST=KVCACHE_OPS_START,
    KVCACHE_OPS_HASHTABLE,
    KVCACHE_OPS_SKIPTABLE,
    KVCACHE_OPS_RBTREE,
    KVCACHE_OPS_BTREE,
    KVCACHE_OPS_END
}KVCACHE_OPS_TYPE;

typedef enum 
{
    KVCACHE_CMD_START=0,
    KVCACHE_CMD_SET=KVCACHE_CMD_START,
    KVCACHE_CMD_GET,
    KVCACHE_CMD_COUNT,
    KVCACHE_CMD_DELETE,
    KVCACHE_CMD_EXIST,
    KVCACHE_CMD_END
}KVCACHE_CMD;

typedef struct
{
    uint8_t* buf;
    int len;
    int free;
    int total;
}elem_t;

typedef struct 
{
    int (*kvcache_set)(void* pthis, elem_t* _key, elem_t* _val);
    elem_t* (*kvcache_get)(void* pthis, const elem_t* _key);
    int (*kvcache_count)(void* pthis, void* args);
    int (*kvcache_delete)(void* pthis, const elem_t* _key);
    int (*kvcache_exist)(void* pthis, const elem_t* _key);
    void* contex;
    char ops_name[128];
}kvcache_ops_t;

#endif // DEBUG