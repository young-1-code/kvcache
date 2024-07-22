#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "mempool.h"
#include "list.h"
#include "kvcache_list.h"
#include "kvcache_execute.h"
#include "threadpool.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct list_t
{
    struct list_t *next;
    int val;
}list_t;

void insert_head(list_t *head, int val)
{
    list_t* node = (list_t*)malloc(sizeof(list_t));
    node->val = val;

    node->next = head->next;
    head->next = node;
}

void free_list(list_t *head)
{
    list_t *tmp, *p = head->next;

    while(p)
    {
        tmp = p->next;
        if(tmp)// && tmp->val%2 == 0)
        {

            printf("00free val=%d\n", tmp->val);
            p->next = tmp->next;

            free(tmp);
        }else {
            p = p->next;
        }

    }
}

void order(list_t *head)
{
    list_t *p = head->next;

    while(p)
    {
        printf("val=%d\n", p->val);
        p = p->next;
    }

}

int main1(void)
{
    list_t *p,  head={.next=NULL, .val=-1};
    p = &head;

    for(int i=0; i<50; ++i){
        insert_head(&head, i);
    }
    free_list(&head);
    order(&head);

    return 0;
}


int main0(int argc, char* argv[])
{
    mempool_t* mpool =  mempool_create();
    uint8_t *p[160000];
    for(int j=0; j<5; j++)
    {
        for(int i=0; i<16*2; ++i)
        {
            p[i] = mpool->alloc(mpool, 1024);
            memset(p[i], 'c', 1024);
        }


        for(int i=0; i<16*2-1; ++i)
            mpool->free(mpool, p[i]);


        mpool->try_compress(mpool);
        printf("***********************\n");
    }

    mempool_destroy(mpool);

    return 0;
}

int task(void *args)
{
    usleep(1000);
    printf("---------split---------------\n");
    return 0;
}

#include "reactor.h"

int main(void)
{
    net_reactor_t* reactor = net_reactor_init();
    threadpool_t* thpool = threadpool_create(4, 1024);

    net_reactor_run(reactor, thpool);

    net_reactor_destory(reactor);
    threadpool_destory(thpool);
    
    return 0;
}