#include <stdlib.h>
#include <string.h>
#include "mempool.h"

static void* _alloc_chunk(void* pthis, int size)
{
    uint8_t *p;
    mempool_t* pool = (mempool_t*)pthis;
    chunklist_t *n, **head = &pool->head;
    n = (chunklist_t*)malloc(size+sizeof(chunklist_t)+1);
    p = (uint8_t*)n;
    p += sizeof(chunklist_t);
    p[0] = ((size/16)-1)/8;
    n->next = *head;
    *head = n;
    return (p+1);
}

static void* _fill_mem_list(void* pthis, int index)
{
    mempool_t* pool = (mempool_t*)pthis;
    union freemem_t  *retp, *n, *curr;
    int elem_n = (index+1)*ALIGN_SIZE;

    uint8_t *tmp, *chunk = (uint8_t*)_alloc_chunk(pthis, LIST_MEM_NUM * (elem_n+1));
    retp = (union freemem_t*)((uint8_t*)chunk+1);
    pool->mem[index] = n = (union freemem_t*)((uint8_t*)chunk+2+elem_n); /* elem 2=1+1 */

    for(int i=1; i<LIST_MEM_NUM; ++i)
    {
        curr = n;
        n = (union freemem_t*)((uint8_t*)n + elem_n+1);
        if(i==(LIST_MEM_NUM-1)) curr->link = NULL;
        else curr->link = n;
    }

    printf("_fill_mem_list -----\n");
    return retp;
}

static void try_compress_sysmem(void* pthis)
{
    uint8_t* mp=NULL;
    int step=0, is_free=0;
    mempool_t* pool = (mempool_t*)pthis;
    chunklist_t *tmp, *p = pool->head; /* 系统内存地址链表 */
    union freemem_t *m_tmp[16];

    while(p)
    {
        tmp = p->next;
        if(tmp){
            is_free = 1;
            mp = ((uint8_t*)tmp)+sizeof(chunklist_t);
            step = mp[0]*8+1;
            mp += 1;
            for(int i=0; i<LIST_MEM_NUM; ++i){
                printf("0x%x ", (*(mp+step*i)));
                m_tmp[i] = (union freemem_t *)(mp+step*i+1);
                if( (*(mp+step*i)&0x80) == 0U ) continue;
                is_free=0; /* 该内存块被占用 */
                break;
            }
            if(is_free){
                /* 删除free list node */
                union freemem_t *n, *curr;
                int index = ((step-1)/8)-1;
                curr = pool->mem[index];
            
                while(curr)
                { 
                    n = curr->link;
                    if(curr >=m_tmp[0] && curr <= m_tmp[15]){
                        pool->mem[index] = curr->link;
                        curr = pool->mem[index];
                    }else if(n >=m_tmp[0] && n <= m_tmp[15]){
                        curr->link = n->link;
                        printf("curr=%p m_tmp[0]=%p m_tmp[15]=%p\n", curr, m_tmp[0],m_tmp[15]);
                    }else{
                        curr = curr->link;
                    }
                }


                printf(" free val=%p\n", tmp);
                p->next = tmp->next;
                free(tmp);
                continue;
            }
            p = p->next;
            printf("------------------------\n");
        }else {
            p = p->next;
        }
    }
}

static void mempool_free(void* pthis, void *p)
{
    mempool_t* pool = (mempool_t*)pthis;
    union freemem_t* tmp;
    uint8_t* _p = p;

    if(!pthis) return;

    _p[-1] &= 0x7f; /* clear 标志位 */
    int index = _p[-1];
    
    tmp = (union freemem_t*)p;
    tmp->link = pool->mem[index];
    pool->mem[index] = tmp;
}

static void* mempool_alloc(void* pthis, size_t size)
{
    mempool_t* pool = (mempool_t*)pthis;
    union freemem_t* retp;
    if(size<=0 || !pthis) return NULL;
    int index = LIST_INDEX( ALIGN(size) );

    retp = pool->mem[index];
    if(!retp) retp = _fill_mem_list(pthis, index);
    else pool->mem[index] = retp->link;

    *(((uint8_t*)retp)-1) = index|0x80; /* set 标志位 */
    return retp;
}


mempool_t* mempool_create(void)
{
    mempool_t* pool = (mempool_t*)malloc(sizeof(mempool_t));
    pool->head = NULL;
    for(int i=0; i<ITEM_MEM_NUM; ++i) {
        pool->mem[i]=NULL;
    }
    pool->alloc = mempool_alloc;
    pool->free = mempool_free;
    pool->try_compress = try_compress_sysmem;

    return pool;
}

void mempool_destroy(mempool_t *pool)
{
    if(!pool) return;

    chunklist_t* tmp, *p = pool->head;
    while(p){
        tmp = p;
        p = p->next;
        free(tmp);
        printf("list\n");
    }
    free(pool);
    pool=NULL;
}

/*****************************slpit*********************************/

void* kvcache_molloc(size_t size)
{
    return malloc(size);
}

void kvcache_free(void *m)
{
    if(m) free(m);
}