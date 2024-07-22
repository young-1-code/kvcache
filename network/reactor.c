#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "server.h"
#include "reactor.h"

void net_event_set( net_event_t*    ev, 
                    int             fd, 
                    event_callbak_f func, 
                    void*           args )
{
    ev->cln_fd = fd;
    ev->args =args;
    ev->callback = func;
    ev->events = 0;
    ev->last_active = time(NULL);
}

int net_event_add(int epfd, int evnets, net_event_t* ev)
{
    int option = -1;
    struct epoll_event epev;
    epev.data.ptr = ev;
    epev.events = ev->events = evnets;

    if(ev->status == 1){
        option = EPOLL_CTL_MOD;
    }else{
        option = EPOLL_CTL_ADD;
        ev->status = 1;
    }

    if(epoll_ctl(epfd, option, ev->cln_fd, &epev) < 0){
        perror("epoll_ctl error!");
        return -1;
    }

    return 0;
}

int net_event_del(int epfd, net_event_t* ev)
{
    struct epoll_event epev={0,{0}};

    if(ev->status != 1) return -1;

    epev.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(epfd, EPOLL_CTL_DEL, ev->cln_fd, &epev);

    return 0;
}

/************************************************/
/*                recv send accpet              */
/************************************************/

int send_callback(int fd, int events, void* args);

int recv_callback(int fd, int events, void* args)
{
    int len = -1;
    net_reactor_t *reactor = (net_reactor_t*)args;
    net_event_t* ev = reactor->events+fd;

    len = recv(fd, ev->datbuf, sizeof(ev->datbuf), 0);
    net_event_del(reactor->epool_fd, ev);

    if(len > 0){
        ev->datlen = len;
        ev->datbuf[len] = '\0';
        net_event_set(ev, fd, send_callback, reactor);
        net_event_add(reactor->epool_fd, EPOLLOUT, ev);
    }else if(len == 0){
        close(ev->cln_fd);
        printf("client close\n");
    }else{
        close(ev->cln_fd);
        printf("recv error!\n");
    }

    return 0;
}

int send_callback(int fd, int events, void* args)
{
    net_reactor_t* reactor = (net_reactor_t*)args;
    net_event_t* ev = reactor->events+fd;

    int len = send(fd, ev->datbuf, ev->datlen, 0);
    if(len > 0){
        printf("send[fd=%d], [%d]%s\n", fd, len, ev->datbuf);
        net_event_del(reactor->epool_fd, ev);
        net_event_set(ev, fd, recv_callback, reactor);
        net_event_add(reactor->epool_fd, EPOLLIN | EPOLLET, ev);
    }else{
        close(ev->cln_fd);
        net_event_del(reactor->epool_fd, ev);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return 0;
}

int accept_callback(int fd, int events, void* args)
{
    net_reactor_t* reactor = (net_reactor_t*)args;
    struct sockaddr_in clientaddr;
    int client_fd = -1, i = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    client_fd = accept(fd, (struct sockaddr*)&clientaddr, &addr_len);
    if(client_fd == -1){
        printf("accept error!\n");
        return -1;
    }

	do {
		for (i = 3;i < MAX_EPOLL_EVENTS; ++i) {
			if (reactor->events[i].status == 0) {
				break;
			}
		}
		if (i == MAX_EPOLL_EVENTS) {
			printf("%s: max connect limit[%d]\n", __func__, MAX_EPOLL_EVENTS);
			break;
		}

		int flag = 0;
		if ((flag = fcntl(client_fd, F_SETFL, O_NONBLOCK)) < 0) {
			printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
			break;
		}

		net_event_set(&reactor->events[client_fd], client_fd, recv_callback, reactor);
		net_event_add(reactor->epool_fd, EPOLLIN, &reactor->events[client_fd]);

	} while (0);

	printf("new connect [%s:%d][time:%ld], pos[%d]\n", 
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), reactor->events[i].last_active, i);

    return 0;
}

int async_net_event_task(void *args)
{
    net_event_t* ev = (net_event_t*)args;
    ev->callback(ev->cln_fd, ev->events, ev->args);
}

/***********************************************************/
/***********************************************************/

int net_reactor_run(net_reactor_t* reactor, threadpool_t* pool)
{
    struct epoll_event events[MAX_EPOLL_EVENTS+1];
    int checkpos=0;

    while(1){
        long now=time(NULL);
        for(int i=0;i<100;i++,checkpos++){
            if(checkpos==MAX_EPOLL_EVENTS){
                checkpos=0;
            }

            if(reactor->events[checkpos].status!=1){
                continue;
            }
            long duration = now - reactor->events[checkpos].last_active;

			if (duration >= 60) {
				close(reactor->events[checkpos].cln_fd);
				printf("[fd=%d] timeout\n", reactor->events[checkpos].cln_fd);
				net_event_del(reactor->epool_fd, &reactor->events[checkpos]);
			}
        }
        int nready=epoll_wait(reactor->epool_fd,events,MAX_EPOLL_EVENTS,1000);
        if(nready<1) continue;

        for(int i=0;i<nready;i++){
            net_event_t* ev=(net_event_t*)events[i].data.ptr;
            printf("ev->events=%d events[i].events=%d\n", ev->events, events[i].events);
            threadpool_add(pool, async_net_event_task, ev);
            // if((events[i].events&EPOLLIN)&&(ev->events&EPOLLIN)){
            //     threadpool_add(pool, async_net_event_task, ev);
            //     // ev->callback(ev->cln_fd, events[i].events, ev->args);
            // }
            // if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
			// 	// ev->callback(ev->cln_fd, events[i].events, ev->args);
            //     threadpool_add(pool, async_net_event_task, ev);
			// }
        }
    }
}

net_reactor_t* net_reactor_init(void)
{
    net_reactor_t* reactor = (net_reactor_t*)malloc(sizeof(net_reactor_t));
    reactor->events = (net_event_t*)malloc(sizeof(net_event_t)*MAX_EPOLL_EVENTS);
    reactor->epool_fd = epoll_create(1);
    if(reactor->epool_fd < 0){
        perror("epoll create!");
        free(reactor->events);
        free(reactor);
        return NULL;
    }

    int server_fd = create_server("192.168.31.89", 23456);
    net_event_set(&reactor->events[server_fd], server_fd, accept_callback, reactor);
    net_event_add(reactor->epool_fd, EPOLLET|EPOLLIN, &reactor->events[server_fd]);

    return reactor;
}

void net_reactor_destory(net_reactor_t* reactor)
{
    if(!reactor) return;
    close(reactor->epool_fd);
    free(reactor->events);
    free(reactor);
}