#include <phosphor-logging/lg2.hpp>
#include <coroutine>
#include "socket.hpp"
#include <iostream>
#include <netinet/in.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char const *argv[])
{
    /* code */
    try
    {
        /* code */
        sdSocket::socket &sk = sdSocket::socket::start(USETCP, USEUDP, SD_SOCKET_PORT);
        sk.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
