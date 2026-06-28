#include "socket.hpp"
#include <sys/epoll.h>
#include <phosphor-logging/lg2.hpp>
#include <cstring>
#include <stdexcept>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <systemd/sd-daemon.h>
#include <unistd.h>
#include <netinet/tcp.h>

namespace sdSocket
{
    inline int setFdFlags(int &fd, int &&flags) noexcept
    {
        int currentFlags = fcntl(fd, F_GETFL, 0);
        if (currentFlags == -1)
            return -1;
        if (currentFlags & flags)
            return 0; // already set
        return fcntl(fd, F_SETFL, currentFlags | flags);
    }

    inline int socketInitialize(const int &socketType, int &port) noexcept
    {
        int fd = -1;
        int fdCount = 0;
        fdCount = sd_listen_fds(0);
        if (fdCount <= 0)
        {
            fd = ::socket(AF_INET, socketType, 0);
            if (fd == -1)
            {
                lg2::error("Failed to create socket: {ERROR}", "ERROR", strerror(errno));
                goto end;
            }
            struct sockaddr_in addr;
            bzero(&addr, sizeof(struct sockaddr_in));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(port);
            if (::bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                lg2::error("Failed to bind socket: {ERROR}", "ERROR", strerror(errno));
                goto clear;
            }
        }
        else
        {
            fd = SD_LISTEN_FDS_START;
            if (sd_is_socket(fd, AF_UNSPEC, SOCK_STREAM, -1) <= 0)
            {
                lg2::error("Failed to set up systemd-passed socket: {ERROR}",
                           "ERROR", strerror(errno));
                goto end;
            }
        }

        setFdFlags(fd, O_NONBLOCK);
        return fd;

    clear:
        ::close(fd);
    end:
        fd = -1;
        return fd;
    }

    // channel implementation: set non-blocking on construction and close on destruction
    channel::channel(int fd) : channelFd(fd)
    {
        readBuf.start = readBuf.end = 0;
        readBuf.buf.resize(4096);
        writeBuf.start = writeBuf.end = 0;
        writeBuf.buf.resize(4096);
        setFdFlags(channelFd, O_NONBLOCK);
    }

    channel::channel(channel &&ch)
    {
        channelFd = ch.channelFd;
        readBuf.start = ch.readBuf.start;
        readBuf.end = ch.readBuf.end;
        readBuf.buf = std::move(ch.readBuf.buf);

        writeBuf.start = ch.writeBuf.start;
        writeBuf.end = ch.writeBuf.end;
        readBuf.buf = std::move(ch.writeBuf.buf);

        ch.channelFd = -1;
    }

    channel::~channel()
    {
        if (channelFd >= 0)
        {
            ::close(channelFd);
            channelFd = -1;
        }
    }

    void channel::send()
    {
        ssize_t total = writeBuf.end - writeBuf.start;
        if (!total)
        {
            return;
        }
        ssize_t sent = 0;
        const uint8_t *buf = writeBuf.buf.data() + writeBuf.start;
        while (sent < total)
        {
            ssize_t n = ::send(channelFd, buf + sent, total - sent, MSG_NOSIGNAL);
            if (n > 0)
            {
                sent += n;
                writeBuf.start += n;
            }
            else if (n == 0)
            {
                break;
            }
            else
            {
                if (errno == EINTR)
                    continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                lg2::error("channel send failed: {ERROR}", "ERROR", strerror(errno));
                break;
            }
        }
    }

    void channel::receive()
    {
        constexpr size_t BUF_SZ = 4096;
        uint8_t buffer[BUF_SZ];
        if (readBuf.start == readBuf.end)
        {
            readBuf.start = readBuf.end = 0;
        }
        while (true)
        {
            ssize_t n = ::recv(channelFd, buffer, BUF_SZ, 0);
            if (n > 0)
            {
                readBuf.buf.insert(readBuf.buf.begin() + readBuf.end, buffer, buffer + n);
                readBuf.end += n;
                lg2::debug("Buf : {BUF}", "BUF", std::string((char *)buffer));
                // continue to drain until EAGAIN
                continue;
            }
            else if (n == 0)
            {
                // peer closed
                break;
            }
            else
            {
                if (errno == EINTR)
                    continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                lg2::error("channel receive failed: {ERROR}", "ERROR", strerror(errno));
                break;
            }
        }
    }

    epoll::epoll()
    {
        epFd = epoll_create1(EPOLL_CLOEXEC);
        if (epFd == -1)
        {
            if (errno == ENOSYS)
            {
                epFd = epoll_create(1);
                if (epFd != -1)
                {
                    setFdFlags(epFd, FD_CLOEXEC);
                }
                else
                {
                    lg2::error("Failed to create epoll instance with epoll_create: {ERROR}", "ERROR", strerror(errno));
                }
            }
            else
            {
                lg2::error("Failed to create epoll instance with epoll_create1: {ERROR}", "ERROR", strerror(errno));
            }
        }
    }

    epoll::~epoll()
    {
        if (epFd != -1)
        {
            close(epFd);
            epFd = -1;
        }
    }

    socket::socket() : isInitialized(false), socketTCPFd(-1), socketUDPFd(-1) {}

    socket &socket::start(int useTcp, int useUdp, int port)
    {
        static socket sk;
        int fd = -1;

        if (!useTcp && !useUdp)
        {
            throw std::runtime_error("TCP or UDP must be enabled.");
        }

        if (sk.isInitialized)
        {
            if ((useTcp && sk.socketTCPFd > 0) || (useUdp && sk.socketUDPFd > 0))
                return sk;
        }

        if (!sk.isEpollValid())
        {
            lg2::error("Failed to create epoll instance, cannot create socket");
            throw std::runtime_error("Failed to create epoll instance");
        }

        if (useTcp)
        {
            fd = socketInitialize(SOCK_STREAM, port);
            if (fd < 0 || (::listen(fd, MAX_EVENTS) < 0))
            {
                throw std::runtime_error("Failed to initialize TCP");
            }
            if (!sk.addChannel(fd, EPOLLIN | EPOLLET))
            {
                ::close(fd);
                throw std::runtime_error("Failed to add channel");
            }
            sk.socketTCPFd = fd;
            lg2::info("Open TCP Success.");
        }

        if (useUdp)
        {
            fd = socketInitialize(SOCK_DGRAM, port);
            if (fd < 0)
            {
                throw std::runtime_error("Failed to initialize UDP");
            }

            if (!sk.addChannel(fd, EPOLLIN | EPOLLET))
            {
                ::close(fd);
                throw std::runtime_error("Failed to add channel");
            }
            sk.socketUDPFd = fd;
            lg2::info("Open UDP Success.");
        }

        sk.isInitialized = true;

        return sk;
    }

    bool socket::addChannel(const int &fd, const uint32_t &events)

    {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        channels.emplace(fd);
        auto ch = std::find_if(channels.begin(), channels.end(),
                               [&](const sdSocket::channel &ch)
                               { return ch.getFd() == fd; });

        if (ch != channels.end())
        {
            ev.events = events;
            ev.data.ptr = (void *)&(*ch);
            if (epoll_ctl(getEpollFd(), EPOLL_CTL_ADD, ch->getFd(), &ev) == -1)
            {
                lg2::error("Failed to add channel to epoll: {ERROR}", "ERROR", strerror(errno));
                return false;
            }
            return true;
        }
        return false;
    }

    void socket::stop()
    {

        if (socketTCPFd >= 0)
        {
            epoll_ctl(getEpollFd(), EPOLL_CTL_DEL, socketTCPFd, NULL);
            ::close(socketTCPFd);
            socketTCPFd = -1;
        }

        if (socketUDPFd >= 0)
        {
            epoll_ctl(getEpollFd(), EPOLL_CTL_DEL, socketUDPFd, NULL);
            ::close(socketUDPFd);
            socketUDPFd = -1;
        }

        isInitialized = false;
    }

    void socket::run()
    {
        struct epoll_event events[MAX_EVENTS];
        char ip[INET_ADDRSTRLEN];
        int n = 0, timeout = -1, clientFd = -1, i = 0;
        channel *ch = NULL;
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        uint32_t fdCount = channels.size();
        while (socketTCPFd > 0 || socketUDPFd > 0)
        {
            if (channels.size() > fdCount)
            {
                timeout = 1000; // 1 second timeout if there are active channels
            }
            else
            {
                timeout = -1; // wait indefinitely if no active channels
            }

            n = ::epoll_wait(getEpollFd(), events, MAX_EVENTS, timeout);

            if (n == -1)
            {
                if (errno == EINTR)
                    continue;
                lg2::error("epoll_wait failed: {ERROR}", "ERROR", strerror(errno));
                break;
            }
            else if (n == 0)
            {
                lg2::debug("epoll_wait timed out, no events received");
                continue;
            }

            for (i = 0; i < n; ++i)
            {
                ch = static_cast<channel *>(events[i].data.ptr);
                if (ch == NULL)
                {
                    continue;
                }

                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                {
                    lg2::info("Client exit.");
                    if (channels.find(*ch) != channels.end())
                    {
                        channels.erase(*ch);
                    }
                    continue;
                }
                else if (events[i].events & EPOLLIN)
                {
                    // handle new connection
                    bzero(&clientAddr, clientAddrLen);
                    if (ch->getFd() == socketTCPFd || ch->getFd() == socketUDPFd)
                    {
                        for (;;)
                        {

                            if (ch->getFd() == socketUDPFd)
                            {
                                ch->receive();
                                break;
                            }
                            else
                            {
                                clientFd = ::accept(socketTCPFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                            }

                            if (clientFd == -1)
                            {
                                if (errno == EAGAIN || errno == EWOULDBLOCK)
                                    break;
                                if (errno == EINTR)
                                    continue;
                                lg2::error("Failed to accept connection: {ERROR}", "ERROR", strerror(errno));
                                break;
                            }

                            setFdFlags(clientFd, O_NONBLOCK);
                            inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
                            lg2::info("connect success,From {IP}", "IP", ip);
                            if (!addChannel(clientFd, EPOLLIN | EPOLLRDHUP))
                            {
                                ::close(clientFd);
                                lg2::error("Failed to add client channel to epoll");
                                continue;
                            }
                        }
                    }
                    else
                    {
                        ch->receive();
                        ch->send();
                    }
                }
                else
                {
                    continue;
                }
            }
        }
    }
}