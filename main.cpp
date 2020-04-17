#include <iostream>
#include "EventLoop.h"
#include <vector>

void SendData(Event *ev);
void RecvData(Event *ev);


#ifdef _WIN64
void RecvData(Event *ev){
    int n = recv(ev->fd, ev->buff, sizeof(ev->buff), 0);
    if(n > 0){
        ev->len = n;
        ev->buff[n] = '\0';
            printf("recv: %s\n", ev->buff);
        ev->Set(ev->fd, SelectEvent::Write,  SendData);
    }else if((n < 0) && (errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)){
        ev->Set(ev->fd, SelectEvent::Read, RecvData);
    } else if(n == 0 || n == -1){
        std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
        closesocket(ev->fd);
        ev->ClearBuffer();
        ev->Del();
    }
}

void SendData(Event *ev)
{
    int ret = -1;
    int Total = 0;
    int lenSend = 0;
    struct timeval tv{};
    tv.tv_sec = 3;
    tv.tv_usec = 500;
    fd_set wset;
    while(true)
    {
        FD_ZERO(&wset);
        FD_SET(ev->fd, &wset);
        if(select(ev->fd + 1, nullptr, &wset, nullptr, &tv) > 0)//3.5秒之内可以send，即socket可以写入
        {
            lenSend = send(ev->fd,ev->buff + Total, ev->len - Total,0);
            if(lenSend == -1)
            {
                ev->ClearBuffer();
                closesocket(ev->fd);
                ev->Del();
                break;
            }
            Total += lenSend;
            if(Total == ev->len)
            {
                ev->ClearBuffer();
                ev->Set(ev->fd, SelectEvent::Read, RecvData);
                break;
            }
        }
        else  //3.5秒之内socket还是不可以写入，认为发送失败
        {
            ev->ClearBuffer();
            closesocket(ev->fd);
            ev->Del();
            break;
        }
    }

}
#else

void RecvData(Event *ev){
    int n = recv(ev->fd, ev->buff, sizeof(ev->buff), 0);
    if(n > 0){
        ev->len = n;
        ev->buff[n] = '\0';
        printf("recv: %s\n", ev->buff);
        ev->Set(ev->fd, EPOLLOUT,  SendData);
    }else if((n < 0) && (errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)){
        ev->Set(ev->fd, EPOLLOUT, RecvData);
    } else if(n == 0 || n == -1){
        std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
        close(ev->fd);
        ev->ClearBuffer();
        ev->Del();
    }
}


void SendData(Event *ev)
{
    int ret = -1;
    int Total = 0;
    int lenSend = 0;
    struct timeval tv{};
    tv.tv_sec = 3;
    tv.tv_usec = 500;
    fd_set wset;
    while(true)
    {
        FD_ZERO(&wset);
        FD_SET(ev->fd, &wset);
        if(select(ev->fd + 1, nullptr, &wset, nullptr, &tv) > 0)//3.5秒之内可以send，即socket可以写入
        {
            lenSend = send(ev->fd,ev->buff + Total, ev->len - Total,0);
            if(lenSend == -1)
            {
                ev->ClearBuffer();
                close(ev->fd);
                ev->Del();
                break;
            }
            Total += lenSend;
            if(Total == ev->len)
            {
                ev->ClearBuffer();
                ev->Set(ev->fd, EPOLLIN, RecvData);
                break;
            }
        }

        else  //3.5秒之内socket还是不可以写入，认为发送失败
        {
            ev->ClearBuffer();
            close(ev->fd);
            ev->Del();
            break;
        }
    }

}

#endif


int main() {

    EventLoop *el = new EventLoop;
    el->InitEvents();
    el->InitEventManger();
    el->customEventManger->On("close", [&](EventManger *, std::vector<pvoid> args){
        auto el = (EventLoop *)args[0];
        el->ShutDown();
    });

    el->CreateEpoll();
    el->LoadEventMap(el->CreateSocket(8888), RecvData);
    el->Run();

    return 0;
}