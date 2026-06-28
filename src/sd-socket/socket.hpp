#pragma once

#include "config.h"

#include <set>
#include <vector>
#include <cstdint>
#include <deque>

namespace sdSocket
{
    class channel
    {
    private:
        int channelFd;
        struct dataBuf
        {
            uint64_t start;
            uint64_t end;
            std::vector<uint8_t> buf;
        };
        struct dataBuf readBuf;
        struct dataBuf writeBuf;

    public:
        channel() = delete;
        explicit channel(int fd);
        channel(channel &&ch);
        ~channel();

        void send();
        void receive();
        int getFd() const { return channelFd; }
    };

    class epoll
    {
    private:
        int epFd;

    public:
        epoll();
        ~epoll();
        bool isEpollValid() const { return epFd != -1; }
        int getEpollFd() const { return epFd; }
    };

    struct Cmp
    {
        bool operator()(const channel &a, const channel &b) const
        {
            if (a.getFd() == b.getFd())
                return false;
            return a.getFd() < b.getFd();
        }
    };

    class socket : public epoll
    {
    private:
        bool isInitialized;
        int socketTCPFd;
        int socketUDPFd;
        std::set<channel, Cmp> channels;

    public:
        static socket &start(int useTcp, int useUdp, int port);
        bool addChannel(const int &fd, const uint32_t &events);
        void stop();
        void run();

    private:
        socket();
        ~socket() = default;
        socket(const socket &) = delete;
        socket &operator=(const socket &) = delete;
        socket(socket &&) = delete;
        socket &operator=(socket &&) = delete;
    };

}
