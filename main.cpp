#include <iostream>
#include "EventLoop.h"

void SendData(Event *ev);
void RecvData(Event *ev);

void RecvData(Event *ev){
    int n = recv(ev->fd, ev->buff, sizeof(ev->buff), 0);
    ev->Del();
    if(n > 0){
        ev->len = n;
        ev->buff[n] = '\0';
            printf("recv: %s\n", ev->buff);
        ev->Set(ev->fd, SelectEvent::Write,  SendData);
    }else if((n < 0) && (errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)){
        ev->Set(ev->fd, SelectEvent::Read, RecvData);
    } else if(n == 0){
        std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
        closesocket(ev->fd);
        ev->ClearBuffer();
        ev->Del();
    }
}

void SendData(Event *ev){
    int n = send(ev->fd, ev->buff, ev->len, 0);
    ev->Del();
    if(n > 0)
    {
        ev->Set(ev->fd, SelectEvent::Read,  RecvData);
    } else {
        std::cout << "[Notify]>> clinet:" << ev->fd << "closed" << std::endl;
        closesocket(ev->fd);
        ev->ClearBuffer();
        ev->Del();
        printf("write error\n");
    }
}


int main() {
    EventLoop el(8888);
    el.BindTcpHandle(RecvData);
    el.Run();
    return 0;
}