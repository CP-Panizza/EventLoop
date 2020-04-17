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


void SendHttp(Event *ev)
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
                close(ev->fd);
                ev->Del();
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

void HandleHttp(Event *ev){
    char buff[1024];
    int n = recv(ev->fd, buff, sizeof(buff), 0);
    if(n > 0){
        buff[n] = '\0';
        printf("recv: %s\n", buff);
        std::string data = R"(HTTP/1.1 200 OK
Date: Sat, 31 Dec 2005 23:59:59 GMT
Content-Type: application/json;charset=utf8

{"name":"cmj", "age": 120})";
        strcpy(ev->buff, data.c_str());
        ev->len = data.size();
        ev->Set(ev->fd, EPOLLOUT,  SendHttp);
    }else if((n < 0) && (errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)){
        ev->Set(ev->fd, EPOLLOUT, HandleHttp);
    } else if(n == 0 || n == -1){
        std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
        close(ev->fd);
        ev->ClearBuffer();
        ev->Del();
    }
}


int main() {


    EventLoop *el = new EventLoop;
    int socket_fd = el->CreateSocket(8888);
    int http_fd = el->CreateSocket(8881);
    el->InitEvents();
    el->InitEventManger();
    el->customEventManger->On("close", [&](EventManger *, std::vector<pvoid> args){
        auto el = (EventLoop *)args[0];
        el->ShutDown();
    });

    el->CreateEpoll();
    el->LoadEventMap(socket_fd, RecvData);
    el->LoadEventMap(http_fd, HandleHttp);
    el->Run();


    return 0;
}