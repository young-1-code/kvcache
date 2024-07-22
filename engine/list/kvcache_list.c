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

    return ret;
}

elem_t* kvcache_list_get(void* pthis, const elem_t* _key)
{
    kvcache_list_ops_t* ops = (kvcache_list_ops_t*)pthis;
    struct list_head *h=&ops->head;
    list_elem_t *pos=NULL;

    list_for_each_entry(pos, h, node){
        if(memcmp(_key->buf, pos->_key->buf, pos->_key->len)!=0) continue;
        log("find %s\n", pos->_val->buf);
        return pos->_val;
    }

    return NULL;
}
int kvcache_list_count(void* pthis, void* args)
{
    int cnt=0;

    return cnt;
}

int kvcache_list_delete(void* pthis, const elem_t* _key)
{
    int ret = failed;

    return ret;
}
int kvcache_list_exist(void* pthis, const elem_t* _key)
{
    int is_exist;

    return is_exist;
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

    return (kvcache_ops_t*)lops;
}

int destroy_kvcache_list_ops(kvcache_ops_t* ops)
{
    kvcache_list_ops_t* lops = (kvcache_list_ops_t*)ops;
    struct list_head *pos, *n, *h = &lops->head;
    list_elem_t *ptr;

    /* delete list node */
    list_for_each_safe(pos, n, h){
        ptr = list_entry(pos, list_elem_t, node);

        log("val=%s key=%s\n", ptr->_val->buf, ptr->_key->buf);
        kvcache_free(ptr);
    }

    kvcache_free(lops);
}