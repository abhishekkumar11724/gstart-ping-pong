#pragma once
#include <cstdint>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

struct Packet
{
    uint8_t type;
    int8_t input;
    int16_t paddleY;
    int16_t ballX, ballY;
    int16_t score1, score2;
};

class Network
{
public:
    bool isHost = false;
    int sockfd = -1;
    sockaddr_in peerAddr{};

    // Host: binds on port and waits for client "hello"
    bool initHost(uint16_t port)
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
            return false;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (::bind(sockfd, (sockaddr *)&addr, sizeof(addr)) < 0)
            return false;
        isHost = true;
        return true;
    }

    // Client: "connects" to hostIP:port so we can use send()/recv()
    bool initClient(const std::string &hostIP, uint16_t port)
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
            return false;
        peerAddr.sin_family = AF_INET;
        peerAddr.sin_port = htons(port);
        inet_pton(AF_INET, hostIP.c_str(), &peerAddr.sin_addr);
        connect(sockfd, (sockaddr *)&peerAddr, sizeof(peerAddr));
        isHost = false;
        return true;
    }

    // send one Packet
    bool sendPacket(const Packet &p)
    {
        return send(sockfd, &p, sizeof(p), 0) == sizeof(p);
    }

    // recv one Packet (blocking=0 will return -1/EAGAIN if none)
    bool recvPacket(Packet &p)
    {
        sockaddr_in from{};
        socklen_t sz = sizeof(from);
        ssize_t r = recvfrom(sockfd, &p, sizeof(p), 0,
                             (sockaddr *)&from, &sz);
        if (r == sizeof(p))
        {
            if (isHost)
                peerAddr = from; // remember client addr on host side
            return true;
        }
        return false;
    }

    ~Network()
    {
        if (sockfd >= 0)
            close(sockfd);
    }
};