#include <stdio.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "server.h"

int create_epoll(int server_fd)
{
    int fd = -1;
    fd = epoll_create(1);

}

int create_server(const char* ip, uint16_t port)
{
    struct sockaddr_in server;  
    int opt = SO_REUSEADDR;
    int fd = -1;
    if(!ip) return -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("socket error!");
        return -1;
    }

    if(fcntl(fd, F_SETFL, O_NONBLOCK) < 0){
        perror("fcntl error!");
        return -1;
    }
  
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt error!");
        return -1;
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);// htonl( INADDR_ANY );

    if(bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0){
        perror("bind error!");
        return -1;
    }

    if(listen(fd, MAX_LISTENTER_NUM) < 0){
        perror("listen error!");
        return -1;
    }

    return fd;
}