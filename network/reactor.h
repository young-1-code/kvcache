#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <stdint.h>
#include "threadpool.h"

#define MAX_EPOLL_EVENTS 1024U

typedef int (*event_callbak_f)(int fd, int events, void* args);

typedef struct net_event_t
{
    int     cln_fd; 
    int     events;
    void    *args;
    int     status;
    uint8_t datbuf[1024];
    int     datlen;
    long    last_active; 
    event_callbak_f callback;
}net_event_t;

typedef struct net_reactor_t
{
    int         epool_fd;
    net_event_t *events;
}net_reactor_t;

net_reactor_t* net_reactor_init(void);
int net_reactor_run(net_reactor_t* reactor, threadpool_t* pool);
void net_reactor_destory(net_reactor_t* reactor);

#endif