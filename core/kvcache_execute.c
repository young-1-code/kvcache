#include "kvcache_execute.h"
#include "mempool.h"
#include "common.h"
#include "kvcache_list.h"

int kvcache_init(void)
{
    s_kvops = kvcache_molloc(sizeof(kvcache_ops_t*) * KVCACHE_OPS_END);
    for(int i=0; i<KVCACHE_OPS_END; ++i) {
        s_kvops[i] = NULL;
    }

    s_kvops[KVCACHE_OPS_LIST] = create_kvcache_list_ops();

    return 0;
}

int kvcache_execute(kvcache_instruct_t* instruct)
{
    int ret=failed;

    if(!instruct || instruct->type > KVCACHE_OPS_END) return failed;
    
    kvcache_ops_t *ops = s_kvops[instruct->type];

    switch (instruct->action)
    {
    case KVCACHE_CMD_SET:{
        /* 1.do action */
        ret = ops->kvcache_set(ops, instruct->_key, instruct->_val);
        /* 2.持久化*/

        /* 3.sync 同步从设备 */

        log("set val %d\n", 1);
        }break;
    case KVCACHE_CMD_GET:{
        elem_t* _val = ops->kvcache_get(ops, instruct->_key);

        log("get val \n%d", 1);
        }break;
    case KVCACHE_CMD_COUNT:{
        int cnt = ops->kvcache_count(ops, NULL);

        log("count val %d", 1);
        }break;
    case KVCACHE_CMD_DELETE:{
        ret = ops->kvcache_delete(ops, instruct->_key);

        log("delete val %d", 1);
        }break;
    case KVCACHE_CMD_EXIST:{
        ret = ops->kvcache_exist(ops, instruct->_key);

        log("exist val %d", 1);
        }break;
    default:{
        ret = failed;
        }break;
    }

    return ret;
}

int kvcache_destroy(void)
{
    int ret=failed;



    return ret;
}