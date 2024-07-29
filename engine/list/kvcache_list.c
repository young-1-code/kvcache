#include <string.h>
#include "kvcache_list.h"
#include "common.h"
#include "mempool.h"

int kvcache_list_set(void* pthis, elem_t* _key, elem_t* _val)
{
    int ret = failed;
    if(!_key || !_val) return ret;
    kvcache_list_ops_t* ops = (kvcache_list_ops_t*)pthis;
    list_elem_t *elem = (list_elem_t*)kvcache_molloc(sizeof(list_elem_t));
    elem->_key = _key;
    elem->_val = _val;
    list_add(&elem->node, &ops->head);
    ++(ops->node_cnt);
    return ret;
}

elem_t* kvcache_list_get(void* pthis, const elem_t* _key)
{
    if( !pthis || !_key ) return NULL;

    kvcache_list_ops_t* ops = (kvcache_list_ops_t*)pthis;
    struct list_head *h=&ops->head;
    list_elem_t *pos=NULL;

    list_for_each_entry(pos, h, node){
        if( _key->len == pos->_key->len && \
            memcmp(_key->buf, pos->_key->buf, _key->len) != 0 )
        {
                continue;
        }
        return pos->_val;
    }

    return NULL;
}
int kvcache_list_count(void* pthis, void* args)
{
    if( !pthis ) return failed;

    return ((kvcache_list_ops_t*)pthis)->node_cnt;
}

int kvcache_list_delete(void* pthis, const elem_t* _key)
{
    if( !pthis || !_key ) return failed;
    kvcache_list_ops_t* ops = (kvcache_list_ops_t*)pthis;
    struct list_head *pos, *n, *h=&ops->head;
    list_elem_t *ptr;

    list_for_each_safe(pos, n, h){
        ptr = list_entry(pos, list_elem_t, node);
        if( ptr->_key->len == _key->len && \
            memcmp(ptr->_key->buf, _key->buf, _key->len) == 0 )
        {
            list_del(pos);
            kvcache_free(ptr->_val);
            kvcache_free(ptr->_key);
            kvcache_free(ptr);
            -- (ops->node_cnt);
            return success;
        }
    }

    return failed;
}
int kvcache_list_exist(void* pthis, const elem_t* _key)
{
    if( kvcache_list_get(pthis, _key) != NULL) return success;
    return failed;
}

int kvcache_list_clear(void* pthis, void* args)
{
    kvcache_list_ops_t* lops = (kvcache_list_ops_t*)pthis;
    struct list_head *pos, *n, *h = &lops->head;
    list_elem_t *ptr;

    /* clear delete list node */
    list_for_each_safe(pos, n, h){
        ptr = list_entry(pos, list_elem_t, node);
        list_del(pos);
        kvcache_free(ptr->_key);
        kvcache_free(ptr->_val);
        kvcache_free(ptr);
        -- (lops->node_cnt);
    }
}


kvcache_ops_t* create_kvcache_list_ops(void)
{
    kvcache_list_ops_t* lops = (kvcache_list_ops_t*)kvcache_molloc(sizeof(kvcache_list_ops_t));
    INIT_LIST_HEAD(&lops->head);
    lops->_ops.contex = NULL;
    
    lops->_ops.kvcache_set = kvcache_list_set;
    lops->_ops.kvcache_get = kvcache_list_get;
    lops->_ops.kvcache_count = kvcache_list_count;
    lops->_ops.kvcache_delete = kvcache_list_delete;
    lops->_ops.kvcache_exist = kvcache_list_exist;
    lops->_ops.kvcache_clear = kvcache_list_clear;

    return (kvcache_ops_t*)lops;
}

int destroy_kvcache_list_ops(kvcache_ops_t* ops)
{
    kvcache_list_clear(ops, NULL);
    kvcache_free(ops);
    return success;
}