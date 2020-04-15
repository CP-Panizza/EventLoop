#include <iostream>
#include "EventLoop.h"

int main() {
    EventLoop el(8888);
    el.Run();
    return 0;
}