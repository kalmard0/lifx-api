#pragma once
#include <string>
#include <cstdint>

namespace lifx {
class Packet;

class Socket {
public:
    static const uint16_t DefaultPort = 56700;
    static Socket* CreateBroadcast(const std::string& broadcastIP,
            uint16_t port = DefaultPort);
    static Socket* CreateStream(const std::string& targetIP, uint16_t port =
            DefaultPort);

    virtual ~Socket() {
    }

    virtual void Send(const Packet& packet) = 0;
    virtual bool Receive(Packet& packet) = 0;
    virtual unsigned GetTicks() const = 0;
};
}
