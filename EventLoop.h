//
// Created by cmj on 20-4-15.
//

#ifndef EVENTLOOP_EVENTLOOP_H
#define EVENTLOOP_EVENTLOOP_H

#include <functional>

#define MAXLINE 4096
#define MAX_COUNT 1024

#include <sys/epoll.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "Event.hpp"

#define pvoid void *

int setnonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}


class EventLoop {
public:
    Event *events;
    int socket_fd;
    int epoll_fd;




    EventLoop(uint16_t port) {
        this->events = new Event[MAX_COUNT + 1];
        socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (socket_fd < 0) {
            std::cout << "[ERROR]>> create socket err!" << std::endl;
            exit(-1);
        }
        int reuse = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        struct sockaddr_in addr;
        bzero(&addr, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        int ret = bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
        if (ret < 0) {
            std::cout << "[ERROR]>> bind error" << std::endl;
            exit(-1);
        }

        ret = listen(socket_fd, 16);

        if (ret < 0) {
            std::cout << "[ERROR]>> listen error" << std::endl;
            exit(-1);
        }
        printf("socket_fd= %d\n", socket_fd);
        epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        Event *accepter = &this->events[MAX_COUNT];
        accepter->SetSrcFd(epoll_fd);
        accepter->Set(socket_fd, EPOLLIN, std::bind(&EventLoop::Accept, this, std::placeholders::_1));

    };

    void RecvData(Event *ev){
        int n = recv(ev->fd, ev->buff, sizeof(ev->buff), 0);
        ev->Del();
        if(n > 0){
            ev->len = n;
            ev->buff[n] = '\0';
            ev->Set(ev->fd, EPOLLOUT, std::bind(&EventLoop::SendData, this, std::placeholders::_1) );
        }

        if(n <= 0){
            std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
            close(ev->fd);
            ev->ClearBuffer();
            ev->Del();
        }
    }


    void SendData(Event *ev){
        int n = write(ev->fd, ev->buff, ev->len);
        printf("nbytes = %d\n", n);
        ev->Del();
        if(n > 0)
        {
            ev->Set(ev->fd, EPOLLOUT,  std::bind(&EventLoop::RecvData, this, std::placeholders::_1) );
        } else {
            std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
            close(ev->fd);
            ev->ClearBuffer();
            ev->Del();
            printf("write error\n");
        }
    }




    void Accept(Event *ev) {
        int i;
        for (i = 0; i < MAX_COUNT; ++i) {
            if (this->events[i].statu == EventStatu::Free) break;
        }

        if (i == MAX_COUNT) {
            std::cout << "[warning]>> max events limited" << std::endl;
            return;
        }

        int connfd = accept(this->socket_fd, NULL, NULL);
        if (connfd < 0) {
            std::cout << "[ERROR]>> accept err" << std::endl;
            return;
        }

        setnonblocking(connfd);
        auto e = &this->events[i];
        e->SetSrcFd(epoll_fd);
        e->Set(connfd, EPOLLIN, std::bind(&EventLoop::RecvData, this, std::placeholders::_1) );
    }


    void Run() {
        struct epoll_event epoll_events[MAX_COUNT + 1];
        int i;
        while (1) {
            int nfd = epoll_wait(this->epoll_fd, epoll_events, MAX_COUNT + 1, 1000);
            for (i = 0; i < nfd; ++i) {
                Event *ev = (Event *) epoll_events[i].data.ptr;
                ev->Call();
            }
        }
    }
};


#endif //EVENTLOOP_EVENTLOOP_H
