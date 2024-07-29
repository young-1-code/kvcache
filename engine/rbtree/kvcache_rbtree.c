#include <string.h> /* for memcmp */
#include "kvcache_rbtree.h"
#include "mempool.h"
#include "common.h"

static int kvcache_rbtree_set(void* pthis, elem_t* _key, elem_t* _val)
{
    if( !pthis || !_key || !_val ) return -1;
    kvcache_rbtree_ops_t *ops = (kvcache_rbtree_ops_t*)pthis;
    rbtree_elem_t *newnode, *pinfo; // 新建结点
    struct rb_node **new = &(ops->root.rb_node), *parent = NULL;
    int status = 0;

    newnode = (rbtree_elem_t*)kvcache_molloc(sizeof(rbtree_elem_t));
    newnode->_key = _key;
    newnode->_val = _val;

    //1.找到插入节点位置
    while (*new)
    {
        pinfo = container_of(*new, rbtree_elem_t, node);

        parent = *new;
        status = memcmp(pinfo->_key->buf, _key->buf, pinfo->_key->len);
        if (status < 0)
            new = &((*new)->rb_left);
        else if (status > 0)
            new = &((*new)->rb_right);
        else
            return failed;
    }

    //2.插入节点
    rb_link_node(&newnode->node, parent, new);
    //3.调整并重新着色
    rb_insert_color(&newnode->node, &(ops->root));
    //4.记录数量
    ++(ops->node_cnt);

    return success;
}

rbtree_elem_t *_kvcache_rbtree_get_node(void* pthis, const elem_t* _key)
{
    if(!pthis || !_key) return NULL;
    kvcache_rbtree_ops_t *ops = (kvcache_rbtree_ops_t*)pthis;
    struct rb_node *rbnode = ops->root.rb_node; /* 根节点 */
    rbtree_elem_t *pinfo=NULL;
    int status = 0;

    while (rbnode!=NULL)
    {
        pinfo = container_of(rbnode, rbtree_elem_t, node);//获取节点信息

        status = memcmp( pinfo->_key->buf, _key->buf, pinfo->_key->len );
        if (status < 0)       /* key值小于当前节点key,往左边节点走 */
            rbnode = rbnode->rb_left;
        else if (status > 0)  /* key值大于当前节点key,往右边节点走 */
            rbnode = rbnode->rb_right;
        else               /* 找到key返回 */
            return pinfo;
    }

    return NULL;
}

static elem_t* kvcache_rbtree_get(void* pthis, const elem_t* _key)
{
    rbtree_elem_t *pinfo = _kvcache_rbtree_get_node(pthis, _key);
    return pinfo ? pinfo->_val : NULL;
}

static int kvcache_rbtree_delete(void* pthis, const elem_t* _key)
{
    kvcache_rbtree_ops_t *ops = (kvcache_rbtree_ops_t*)pthis;
    rbtree_elem_t *rnode=NULL;

    if( !pthis || !_key ) return -1;

    /* 在红黑树中查找key对应的节点tmp */
    rnode = _kvcache_rbtree_get_node(pthis, _key);
    if (rnode == NULL){
        return failed;
    }
    /* 从红黑树中删除节点tmp */
    rb_erase(&(rnode->node), &(ops->root));
    
    kvcache_free(rnode->_val);
    kvcache_free(rnode->_key);
    kvcache_free(rnode);
    --(ops->node_cnt);

    return success;
}

static int kvcache_rbtree_exist(void* pthis, const elem_t* _key)
{
    kvcache_rbtree_ops_t *ops = (kvcache_rbtree_ops_t*)pthis;
    rbtree_elem_t *node=NULL;

    if( !pthis || !_key ) return -1;

    node = _kvcache_rbtree_get_node(pthis, _key);

    return node ? success : failed;
}

static int kvcache_rbtree_count(void* pthis, void* args)
{
    if( !pthis ) return failed;
    return ((kvcache_rbtree_ops_t*)pthis)->node_cnt;
}

static int kvcache_rbtree_clear(void* pthis, void* args)
{
    
}

kvcache_ops_t* create_kvcache_rbtree_ops(void)
{
    kvcache_rbtree_ops_t* ops = (kvcache_rbtree_ops_t*)kvcache_molloc(sizeof(kvcache_rbtree_ops_t));
    ops->_ops.kvcache_set = kvcache_rbtree_set;
    ops->_ops.kvcache_get = kvcache_rbtree_get;
    ops->_ops.kvcache_exist = kvcache_rbtree_exist;
    ops->_ops.kvcache_delete = kvcache_rbtree_delete;
    ops->_ops.kvcache_count = kvcache_rbtree_count;
    ops->_ops.kvcache_clear = kvcache_rbtree_clear;
    snprintf(ops->_ops.ops_name, 128, "rbtree kvcache");
    ops->node_cnt = 0;

    ops->root = RB_ROOT;
    return (kvcache_ops_t*)ops;
}

int destroy_kvcache_rbtree_ops(kvcache_ops_t* ops)
{

    kvcache_free(ops);
}