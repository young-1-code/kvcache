#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<errno.h>


#define BUFFER_LENGTH 4096
#define MAX_EPOLL_EVENTS 1024
#define SERVER_PORT 8888

typedef int NCALLBACK(int,int,void*);



struct ntyevent{
    int fd;
    int events;
    void* arg;
    int (*callback)(int fd,int events,void* arg);
    int status;
    char buffer[BUFFER_LENGTH];
    int length;
    long last_active;
};


struct ntyreactor{
    int epfd;
    ntyevent* events;
};

int recv_cb(int fd, int events, void *arg);
int send_cb(int fd, int events, void *arg);
void nty_event_set(ntyevent* ev,int fd,NCALLBACK callback,void* arg){
    ev->fd=fd;
    ev->callback=callback;
    ev->events=0;
    ev->arg=arg;
    ev->last_active=time(NULL);
}
int nty_event_add(int epfd,int events,ntyevent* ev){
    epoll_event ep_ev;
    ep_ev.data.ptr=ev;
    ep_ev.events = ev->events = events;
    
    int op;
    if(ev->status==1){
        op=EPOLL_CTL_MOD;
    }else{
        op=EPOLL_CTL_ADD;
        ev->status=1;
    }
    if(epoll_ctl(epfd,op,ev->fd,&ep_ev)<0){
        printf("epoll add faild\n");
        return -1;
    }
    return 0;
}

int nty_event_del(int epfd, struct ntyevent *ev) {

	struct epoll_event ep_ev = {0, {0}};

	if (ev->status != 1) {
		return -1;
	}

	ep_ev.data.ptr = ev;
	ev->status = 0;
	epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &ep_ev);

	return 0;
}

int recv_cb(int fd, int events, void *arg) {

	struct ntyreactor *reactor = (struct ntyreactor*)arg;
	struct ntyevent *ev = reactor->events+fd;

	int len = recv(fd, ev->buffer, BUFFER_LENGTH, 0);
	nty_event_del(reactor->epfd, ev);

	if (len > 0) {
		
		ev->length = len;
		ev->buffer[len] = '\0';

		printf("C[%d]:%s\n", fd, ev->buffer);

		nty_event_set(ev, fd, send_cb, reactor);
		nty_event_add(reactor->epfd, EPOLLOUT, ev);
		
		
	} else if (len == 0) {

		close(ev->fd);
		printf("[fd=%d] pos[%ld], closed\n", fd, ev-reactor->events);
		 
	} else {

		close(ev->fd);
		printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
		
	}

	return len;
}


int send_cb(int fd, int events, void *arg) {

	struct ntyreactor *reactor = (struct ntyreactor*)arg;
	struct ntyevent *ev = reactor->events+fd;

	int len = send(fd, ev->buffer, ev->length, 0);
	if (len > 0) {
		printf("send[fd=%d], [%d]%s\n", fd, len, ev->buffer);

		nty_event_del(reactor->epfd, ev);//删掉又恢复,重置时间,(因为超时会断开连接)
		nty_event_set(ev, fd, recv_cb, reactor);
		nty_event_add(reactor->epfd, EPOLLIN, ev);
		
	} else {

		close(ev->fd);

		nty_event_del(reactor->epfd, ev);
		printf("send[fd=%d] error %s\n", fd, strerror(errno));

	}

	return len;
}

int accept_cb(int fd, int events, void *arg) {

	struct ntyreactor *reactor = (struct ntyreactor*)arg;
	if (reactor == NULL) return -1;

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int clientfd;

	if ((clientfd = accept(fd, (struct sockaddr*)&client_addr, &len)) == -1) {
		if (errno != EAGAIN && errno != EINTR) {
			
		}
		printf("accept: %s\n", strerror(errno));
		return -1;
	}

	int i = 0;
	do {
		
		for (i = 3;i < MAX_EPOLL_EVENTS;i ++) {
			if (reactor->events[i].status == 0) {
				break;
			}
		}
		if (i == MAX_EPOLL_EVENTS) {
			printf("%s: max connect limit[%d]\n", __func__, MAX_EPOLL_EVENTS);
			break;
		}

		int flag = 0;
		if ((flag = fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) {
			printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
			break;
		}

		nty_event_set(&reactor->events[clientfd], clientfd, recv_cb, reactor);
		nty_event_add(reactor->epfd, EPOLLIN, &reactor->events[clientfd]);

	} while (0);

	printf("new connect [%s:%d][time:%ld], pos[%d]\n", 
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), reactor->events[i].last_active, i);

	return 0;

}

int init_sockfd(uint16_t port){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    fcntl(sockfd,F_SETFL,O_NONBLOCK);
    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(port);
    
    bind(sockfd,(sockaddr*)&server_addr,sizeof(sockaddr_in));
    if(listen(sockfd,20)<0){
        perror("listen");
    }
    return sockfd;
}

int ntyreactor_init(ntyreactor* reactor){
    if(reactor==NULL) return -1;
    memset(reactor,0,sizeof(ntyreactor));
    reactor->epfd=epoll_create(1);
    if(reactor->epfd<=0){
        // perror("create epfd in%s",__func__);
        close(reactor->epfd);
        return -2;
    }
    reactor->events=(ntyevent*)malloc(sizeof(ntyevent)*MAX_EPOLL_EVENTS);
    memset(reactor->events,0,sizeof(ntyevent)*MAX_EPOLL_EVENTS);
    if(reactor->events==NULL){
        // perror("create epfd in%s",__func__);
        close(reactor->epfd);
        return -3;
    }
}

int ntyreactor_destory(ntyreactor *reactor) {

	close(reactor->epfd);
	free(reactor->events);

}
int ntyreactor_addlistener(ntyreactor* reactor,int sockfd,NCALLBACK* acceptor){
    if(reactor==NULL) return -1;
    if(reactor->events==NULL) return -1;
    nty_event_set(&reactor->events[sockfd],sockfd,acceptor,reactor);
    nty_event_add(reactor->epfd,EPOLLIN,&reactor->events[sockfd]);
    return 0;
}



int ntyreactor_run(ntyreactor* reactor){
    if(reactor==NULL) return -1;
    if(reactor->epfd<0) return -1;
    if(reactor->events==NULL) return -1;
    epoll_event events[MAX_EPOLL_EVENTS+1];

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
				close(reactor->events[checkpos].fd);
				printf("[fd=%d] timeout\n", reactor->events[checkpos].fd);
				nty_event_del(reactor->epfd, &reactor->events[checkpos]);
			}
        }
        int nready=epoll_wait(reactor->epfd,events,MAX_EPOLL_EVENTS,1000);
        if(nready<0) continue;
        for(int i=0;i<nready;i++){
            ntyevent* ev=(ntyevent*)events[i].data.ptr;
            if((events[i].events&EPOLLIN)&&(ev->events&EPOLLIN)){
                ev->callback(ev->fd,events[i].events,ev->arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
				ev->callback(ev->fd, events[i].events, ev->arg);
			}
        }
    }
}



int main(int argc,char** argv){
    uint16_t port=SERVER_PORT;
    if(argc==2){
        port=atoi(argv[1]);
    }
    int sockfd=init_sockfd(port);
   
    ntyreactor* reactor=(ntyreactor*)malloc(sizeof(ntyreactor));
    ntyreactor_init(reactor);
    ntyreactor_addlistener(reactor,sockfd,accept_cb);


    ntyreactor_run(reactor);

    ntyreactor_destory(reactor);
    close(sockfd);

    return 0;

}
