//
// Created by cmj on 20-4-15.
//

#ifndef EVENTLOOP_EVENT_H
#define EVENTLOOP_EVENT_H


#include "EventLoop.h"
#define pvoid void *

enum EventStatu {
    Free,
    Using
};


class Event {
public:
    typedef std::function<void(Event *)> CallBack;
    int src_fd; //事件源fd exp: select_fd, epoll_fd
    pvoid data;
    int fd;
    int events;
    char buff[MAXLINE];
    int len;
    CallBack callBack;
    EventStatu statu = EventStatu::Free;

    void SetSrcFd(int _src_fd){
        this->src_fd = _src_fd;
    }


    void Set(int _fd, int _event, CallBack _callback) {
        this->fd = _fd;
        this->events = _event;
        this->callBack = _callback;

        struct epoll_event epv;
        memset(&epv, 0, sizeof(epv));
        epv.events = static_cast<uint32_t>(this->events);
        epv.data.ptr = this;
        int op;

        if (this->statu == EventStatu::Using) {
            op = EPOLL_CTL_MOD;
        } else {
            op = EPOLL_CTL_ADD;
            this->statu = EventStatu ::Using;
        }

        if (epoll_ctl(this->src_fd, op, this->fd, &epv) < 0) {
            printf("epoll_ctl failed\n");
        } else {
            printf("epoll_ctl [fd = %d] events[%0x] success\n", this->fd, this->events);
        }
    }

    void Del(){
        if(this->statu == EventStatu::Free) return;
        epoll_ctl(this->src_fd, EPOLL_CTL_DEL, this->fd, nullptr);
        this->statu = EventStatu ::Free;
    }

    void ClearBuffer(){
        memset(this->buff, 0, sizeof(this->buff));
        this->len = 0;
    }

    void Call() {
        callBack(this);
    }
};



#endif //EVENTLOOP_EVENT_H
