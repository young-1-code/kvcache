#include <string.h>
#include "kvcache_hashtable.h"
#include "common.h"
#include "mempool.h"

size_t calc_hash_val(const elem_t* _key)
{
    unsigned long i=0, __h = 0; 

    for( ; i<_key->len; ++i ){
        __h = 5*__h + _key->buf[i];
    }

    return __h;
}

static int _kvcache_copy_hashtabel(struct htable_t *src, struct htable_t *desc)
{
    struct hnode_t  *snode, **dnode, *tmp, *ttmp;
    size_t hkey;

    for( int i=0; i<src->bucket_n; ++i ){
        snode = src->tb[i];
        if( !snode ) continue;

        while(snode) {
            hkey = desc->hash_fn(snode->_key) % desc->bucket_n;
            dnode  = &(desc->tb[hkey]);
            if( NULL == *dnode ){
                ttmp = snode->next;
                *dnode = snode;
                snode->next = NULL;
                snode = ttmp;
            }else{
                tmp = (*dnode)->next;
                (*dnode)->next = snode;
                ttmp = snode->next; /* 临时保存snode节点的next */
                snode->next = tmp;
                snode = ttmp;
            }
            ++(desc->node_n);
        }
    }

    return success;
}


static int _kvcache_rehash(void* pthis, int status)
{
    int index;
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)pthis;
    if( !hops ) return failed;

    if(status == 0){ /* inc */
        index = hops->htable[0]->bucket_index + 1;
        if(index > HASH_TABLE_BUCKET_CNT_LEVEL) return failed;
    }else{ /* desc */
        index = hops->htable[0]->bucket_index - 1;
        if( index < 0 ) return failed; 
    }
    hops->htable[1] = (struct htable_t*)kvcache_molloc(sizeof(struct htable_t));
    hops->htable[1]->bucket_index = index;
    hops->htable[1]->bucket_n = bucket_cnt_array[ index ];
    hops->htable[1]->hash_fn = calc_hash_val; /* 可能变化 */
    hops->htable[1]->node_n = 0;
    hops->htable[1]->tb = (struct hnode_t**)kvcache_molloc(sizeof(struct hnode_t*) * hops->htable[1]->bucket_n);
    memset(hops->htable[1]->tb, 0, sizeof(struct hnode_t*) * hops->htable[1]->bucket_n);

    _kvcache_copy_hashtabel(hops->htable[0], hops->htable[1]);
    hops->htable[1]->factor = hops->htable[1]->node_n*1.0 / hops->htable[1]->bucket_n;

    kvcache_free(hops->htable[0]->tb);
    kvcache_free(hops->htable[0]);

    hops->htable[0] = hops->htable[1];
    hops->htable[1] = NULL;

    return success;
}

static int kvcache_hashtable_set(void* pthis, elem_t* _key, elem_t* _val)
{
    if( !pthis || !_key || !_val ) return -1;
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)pthis;
    size_t hkey = hops->htable[0]->hash_fn(_key) % hops->htable[0]->bucket_n;

    struct hnode_t* newnode = kvcache_molloc(sizeof(struct hnode_t));
    newnode->next = NULL;
    newnode->_key = _key;
    newnode->_val = _val;

    struct hnode_t *tmp, **node = &(hops->htable[0]->tb[hkey]);
    if( NULL == *node ) {
        *node = newnode; 
    } else {
        tmp = (*node)->next;
        (*node)->next = newnode;
        newnode->next = tmp;
    }
    ++(hops->htable[0]->node_n);
    hops->htable[0]->factor = hops->htable[0]->node_n*1.0 / hops->htable[0]->bucket_n;
    if( hops->htable[0]->factor > INC_LOAD_FACTOR_MAX) 
        _kvcache_rehash(pthis, 0);

    return 0;
}

static elem_t* kvcache_hashtable_get(void* pthis, const elem_t* _key)
{
    if( !pthis || !_key ) return NULL;
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)pthis;
    size_t hkey = hops->htable[0]->hash_fn(_key) % hops->htable[0]->bucket_n;
    struct hnode_t *node = (hops->htable[0]->tb[hkey]);

    if( !node ) return NULL;

    while( node )
    {
        if( node->_key->len==_key->len && \
            memcmp(node->_key->buf, _key->buf, _key->len) == 0 )
        {
            return node->_val;
        }

        node = node->next;
    }

    return NULL;
}

static int kvcache_hashtable_delete(void* pthis, const elem_t* _key)
{
    if( !pthis || !_key ) return failed;
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)pthis;
    size_t hkey = hops->htable[0]->hash_fn(_key) % hops->htable[0]->bucket_n;
    struct hnode_t *tmp, *node = (hops->htable[0]->tb[hkey]);
    
    if( !node ) return failed;

    do {
        if( node->_key->len == _key->len && \
            memcmp(node->_key->buf, _key->buf, _key->len) == 0 )
        {
            hops->htable[0]->tb[hkey] = hops->htable[0]->tb[hkey]->next;
            tmp = node;
            break;
        }

        while( node->next )
        {
            tmp = node->next;
            if( tmp->_key->len == _key->len && \
                memcmp(tmp->_key->buf, _key->buf, _key->len) == 0 )
            {
                node->next = tmp->next;
                break;
            }

            node = node->next;
        }
    }while(0U);

    kvcache_free(tmp->_val);
    kvcache_free(tmp->_key);
    kvcache_free(tmp);

    --(hops->htable[0]->node_n);
    hops->htable[0]->factor = hops->htable[0]->node_n*1.0 / hops->htable[0]->bucket_n;
    if( hops->htable[0]->factor < DEC_LOAD_FACTOR_MIN) 
        _kvcache_rehash(pthis, 1);

    return success;
}

static int kvcache_hashtable_exist(void* pthis, const elem_t* _key)
{
    return kvcache_hashtable_get(pthis, _key)!=NULL ? success : failed;
}

static int kvcache_hashtable_count(void* pthis, void* args)
{
    if( !pthis ) return failed;
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)pthis;
    return hops->htable[0]->node_n;
}

static int kvcache_hashtable_clear(void* pthis, void* args)
{

}


kvcache_ops_t* create_kvcache_hashtable_ops(void)
{
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)kvcache_molloc(sizeof(kvcache_htable_ops_t));
    hops->htable[0] = kvcache_molloc( sizeof(struct htable_t) );
    hops->htable[0]->bucket_index=0;
    hops->htable[0]->bucket_n = bucket_cnt_array[0];
    hops->htable[0]->factor = 0.0;
    hops->htable[0]->hash_fn = calc_hash_val;
    hops->htable[0]->node_n = 0;
    hops->htable[0]->tb = (struct hnode_t **)kvcache_molloc(sizeof(struct hnode_t*) * hops->htable[0]->bucket_n);
    memset(hops->htable[0]->tb, 0, sizeof(struct hnode_t*) * hops->htable[0]->bucket_n);
    
    hops->htable[1] = NULL;

    hops->_ops.contex = hops;
    hops->_ops.kvcache_clear  = kvcache_hashtable_clear;
    hops->_ops.kvcache_count  = kvcache_hashtable_count;
    hops->_ops.kvcache_delete = kvcache_hashtable_delete;
    hops->_ops.kvcache_exist  = kvcache_hashtable_exist;
    hops->_ops.kvcache_get    = kvcache_hashtable_get;
    hops->_ops.kvcache_set    = kvcache_hashtable_set;
    snprintf(hops->_ops.ops_name, 128, "hashtable kvcache");

    return (kvcache_ops_t*)hops;
}

int destroy_kvcache_hashtable_ops(kvcache_ops_t* ops)
{
    if( !ops ) return failed;
    kvcache_htable_ops_t* hops = (kvcache_htable_ops_t*)ops;

    kvcache_free( hops->htable[0]->tb );
    kvcache_free( hops->htable[0] );
    kvcache_free( hops );

    return success;
}