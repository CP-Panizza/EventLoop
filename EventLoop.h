//
// Created by cmj on 20-4-15.
//

#ifndef EVENTLOOP_EVENTLOOP_H
#define EVENTLOOP_EVENTLOOP_H

#include <functional>

#define MAXLINE 4096
#define MAX_COUNT 1024

#include "socket_header.h"


#include <string.h>


#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "Event.hpp"

#define pvoid void *
#ifdef _WIN64

int setnonblocking( SOCKET s){
    unsigned long ul=1;
    int ret=ioctlsocket(s, FIONBIO, &ul);//设置成非阻塞模式。
    if(ret==SOCKET_ERROR)//设置失败。
    {
        return -1;
    }
    return 1;
}

#else
int setnonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

#endif

class EventLoop {
public:
    Event *events;
    Event::CallBack tcp_call_back;
#ifndef _WIN64
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

#else

public:
    SOCKET socket_fd;
    int max_fd;
    FDS fds;
    EventLoop(uint16_t port){
        this->events = new Event[MAX_COUNT + 1];
        WORD dwVersion = MAKEWORD(2, 2);
        WSAData wsaData{};
        WSAStartup(dwVersion, &wsaData);
        sockaddr_in servaddr{};
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET; //网络类型
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port); //端口

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("[ERROR]>> create socket error: %s(errno: %d)\n", strerror(errno), errno);
            WSACleanup();
            exit(-1);
        }

        bool bReAddr = true;
        if (SOCKET_ERROR == (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &bReAddr, sizeof(bReAddr)))) {
            std::cout << "[ERROR]>> set resueaddr socket err!" << std::endl;
            WSACleanup();
            exit(-1);
        }

        if (bind(socket_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == INVALID_SOCKET) {
            printf("[ERROR]>> bind socket error: %s(errno: %d)\n", strerror(errno), errno);
            WSACleanup();
            exit(-1);
        }

        //监听，设置最大连接数10
        if (listen(socket_fd, 10) == INVALID_SOCKET) {
            printf("[ERROR]>> listen socket error: %s(errno: %d)\n", strerror(errno), errno);
            WSACleanup();
            exit(-1);
        }
        max_fd = socket_fd;
        if(setnonblocking(socket_fd) == -1){
            printf("[ERROR]>> set socket_fd nnonblock err");
            exit(-1);
        }

        FD_ZERO(&fds.read_fd);
        FD_ZERO(&fds.write_fd);
        FD_ZERO(&fds._read_fd);
        FD_ZERO(&fds._read_fd);
        FD_SET(socket_fd, &fds.read_fd);
        Event *accepter = &this->events[MAX_COUNT];
        accepter->SetSrcFd(&fds);
        accepter->Set(socket_fd, SelectEvent::Read, std::bind(&EventLoop::Accept, this, std::placeholders::_1));
    }

    int max(int a, int b){
        if(a == b) return a;
        return a > b? a : b;
    }

    void BindTcpHandle(const Event::CallBack &callBack){
        tcp_call_back = callBack;
    }

    void Accept(Event *ev) {
        int i;
        for (i = 0; i < MAX_COUNT; ++i) {
            if (this->events[i].statu == EventStatu::Free) break;
        }

        printf("find place %d", i);
        if (i == MAX_COUNT) {
            std::cout << "[warning]>> max events limited" << std::endl;
            return;
        }

        SOCKET connfd = accept(this->socket_fd, NULL, NULL);
        if (connfd < 0) {
            std::cout << "[ERROR]>> accept err" << std::endl;
            exit(-1);
        }

        printf("accept a clinet %d\n", connfd);
        if(setnonblocking(connfd) == -1){
            printf("[ERROR]>> clinet %d set nonblock err\n", connfd);
            closesocket(connfd);
            return;
        }
        max_fd = max(socket_fd, connfd);
        auto e = &this->events[i];
        e->SetSrcFd(&this->fds);
        e->Set(connfd, SelectEvent::Read, this->tcp_call_back);
    }




    int GetEventByFd(SOCKET s){
        int i;
        for (i = 0; i <= MAX_COUNT; ++i) {
            if (this->events[i].fd == s && this->events[i].statu == EventStatu::Using) break;
        }

        if (i > MAX_COUNT) {
            std::cout << "[warning]>> max events limited" << std::endl;
            return -1;
        } else {
            return i;
        }
    }


    void Run() {
        int ret;
        struct timeval t = {5, 0};
        while (1) {
            FD_ZERO(&this->fds._write_fd);
            FD_ZERO(&this->fds._read_fd);
            this->fds._read_fd = this->fds.read_fd;
            this->fds._write_fd = this->fds.write_fd;
            printf("select before read_count:%d\n", this->fds.read_fd.fd_count);
            ret = select(max_fd + 1, &this->fds._read_fd, &this->fds._write_fd, NULL, &t);//最后一个参数为NULL，一直等待，直到有数据过来,客户端断开也会触发读/写状态，然后判断recv返回值是否为0，为0这说明客户端断开连接
            printf("select after read_count:%d\n", this->fds.read_fd.fd_count);
            if (ret == SOCKET_ERROR) {
                printf("[ERROR]>> select err!");
                WSACleanup();
                exit(-1);
            }

            for (int i = 0; i < ret; ++i) {
                SOCKET s = this->fds._read_fd.fd_array[i];
                if (FD_ISSET(s, &this->fds._read_fd)) {
                    int index = GetEventByFd(s);
                    if(index == -1){
                        printf("[ERROR]>> find Event Err!");
                        WSACleanup();
                        exit(-1);
                    }
                    ((Event *)(&this->events[index]))->Call();
                }
            }

            for (int i = 0; i < ret; ++i) {
                SOCKET s = this->fds._write_fd.fd_array[i];
                if (FD_ISSET(s, &this->fds._write_fd)) {
                    int index = GetEventByFd(s);
                    if(index == -1){
                        printf("[ERROR]>> find Event Err!");
                        exit(-1);
                    }
                    ((Event *)(&this->events[index]))->Call();
                }
            }

        }
    }
#endif
};


#endif //EVENTLOOP_EVENTLOOP_H
