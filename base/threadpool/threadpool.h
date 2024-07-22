#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include "list.h"

/* 任务线程状态 1:runing 0:finish */
static __thread int g_tls_state; 

typedef int (*taskcb_f)(void* args);

typedef struct taskque_t
{
    struct list_head node;
    taskcb_f func;
    void *args;
}taskque_t;

typedef struct threadpool_t
{
    int curr_thread_n; /* 当前池中线程个数      */
    int max_ts_n;      /* 池中允许的最大线程个数 */
    int curr_ts_n;     /* 当前任务个数          */
    struct list_head head; /* 任务队列              */
    pthread_mutex_t  lock; /* 互斥锁             */
    pthread_cond_t   cond; /* 唤醒线程的条件变量  */
}threadpool_t;

threadpool_t* threadpool_create(int thread_num, int maxts_num);

int threadpool_add(threadpool_t* pool, taskcb_f func, void* args);

int threadpool_urgency_add(threadpool_t* pool, taskcb_f func, void* args);

int threadpool_adjust(threadpool_t* pool, int thread_num);

int threadpool_destory(threadpool_t* pool);

#endif // !__THREADPOOL_H__
