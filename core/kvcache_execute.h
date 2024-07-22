#ifndef __KVCACHE_EXECUTE_H__
#define __KVCACHE_EXECUTE_H__

#include <stdint.h>
#include "kvcache_ops.h"

typedef struct
{
    KVCACHE_OPS_TYPE      type;    /* 类型 eg: list hash... */
    KVCACHE_CMD           action;  /* 动作 eg: SET GET... */
    elem_t*              _key;     /* 键值 */
    elem_t*              _val;
}kvcache_instruct_t;

static kvcache_ops_t **s_kvops;

int kvcache_init(void);
int kvcache_execute(kvcache_instruct_t* instruct);
int kvcache_destroy(void);

#endif // !__KVCACHE_EXECUTE_H__
