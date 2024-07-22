#include "threadpool.h"
#include <unistd.h>
#include <stdlib.h>

int _exit_thread(void *args)
{
    int *done = (int*)args;
    g_tls_state = 0;
    *done = 1;
    return 0;
}

void* _threadpool_work(void* args)
{
    taskque_t *que=NULL;
    struct list_head *pos=NULL;
    threadpool_t* pool = (threadpool_t*)args;

    if(!pool) return NULL;

    g_tls_state = 1; /* 运行状态 */

    while(g_tls_state)
    {
        pthread_mutex_lock(&(pool->lock));
        while(list_empty(&(pool->head))){
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        }
        pos = pool->head.next;
        list_del(pos);
        --pool->curr_ts_n;
        pthread_mutex_unlock(&(pool->lock));

        que = list_entry(pos, taskque_t, node);
        que->func(que->args);
        free(que);
    }

}

int exit_one_thread(threadpool_t* pool)
{
    if(!pool) return -1;

    int done=0;
    if(0 != threadpool_urgency_add(pool, _exit_thread, &done) ) 
        return -1;

    while(!done){
        usleep(10);
        continue;
    }
    pthread_mutex_lock(&(pool->lock));
    --pool->curr_thread_n;
    pthread_mutex_unlock(&(pool->lock));

    pthread_cond_signal(&(pool->cond));

    return 0;
}

int add_one_thread(threadpool_t* pool)
{
    pthread_t th;
    if(!pool) return -1;

    pthread_create(&th, NULL, _threadpool_work, pool);
    pthread_detach(th);

    pthread_mutex_lock(&(pool->lock));
    ++pool->curr_thread_n;
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

int threadpool_urgency_add(threadpool_t* pool, taskcb_f func, void* args)
{
    if(!pool || !func) return -1;
    if(pool->curr_ts_n >= pool->max_ts_n) return -2;

    taskque_t *que = (taskque_t*)malloc(sizeof(taskque_t));
    if(!que) return -3;
    pthread_mutex_lock(&(pool->lock));
    que->args = args;
    que->func = func;
    list_add(&(que->node), &(pool->head));
    ++pool->curr_ts_n;
    pthread_mutex_unlock(&(pool->lock));

    pthread_cond_signal(&(pool->cond));

    return 0;
}

int threadpool_add(threadpool_t* pool, taskcb_f func, void* args)
{
    if(!pool || !func) return -1;
    if(pool->curr_ts_n >= pool->max_ts_n) return -2;

    taskque_t *que = (taskque_t*)malloc(sizeof(taskque_t));
    if(!que) return -3;
    pthread_mutex_lock(&(pool->lock));
    que->args = args;
    que->func = func;
    list_add_tail(&(que->node), &(pool->head));
    ++pool->curr_ts_n;
    pthread_mutex_unlock(&(pool->lock));
    
    pthread_cond_signal(&(pool->cond));

    return 0;
}

int threadpool_adjust(threadpool_t* pool, int thread_num)
{
    if(!pool || thread_num < 0) return -1;

    int curr_thread_n = pool->curr_thread_n;

    for(int i=curr_thread_n; i<thread_num; ++i){
        add_one_thread(pool);
    }

    for(int i=thread_num; i<curr_thread_n; ++i){
        exit_one_thread(pool);
    }

    return 0;
}   

int threadpool_destory(threadpool_t* pool)
{
    if(!pool) return -1;

    int curr_thread_n = pool->curr_thread_n;

    for(int i=0; i<curr_thread_n; ++i){
        exit_one_thread(pool);
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
    free(pool);

    return 0;
}

threadpool_t* threadpool_create(int thread_num, int maxts_num)
{
    threadpool_t* pool;
    pthread_t th;

    g_tls_state = 1;
    if(thread_num<1 || maxts_num < 1) return NULL;

    pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if(!pool) return NULL;

    pool->curr_thread_n = thread_num;
    pool->max_ts_n = maxts_num;
    pool->curr_ts_n = 0;
    INIT_LIST_HEAD(&pool->head);

    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->cond), NULL);


    /* create thread work */
    for(int i=0; i<pool->curr_thread_n; ++i){
        pthread_create(&th, NULL, _threadpool_work, pool);
        pthread_detach(th);
    }

    return pool;
}