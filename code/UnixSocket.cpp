#include <iostream>
#include <cassert>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "Socket.h"
#include "Packet.h"

namespace lifx {
class UnixSocket: public Socket {
public:
    UnixSocket(const std::string& ip, uint16_t port, bool broadcast) {
        if (!broadcast) {
            assert(!"tcp unsupported");
        }

        struct sockaddr_in write_addr;
        struct sockaddr_in read_addr;
        struct hostent *server;

        write_sock = socket(AF_INET, SOCK_DGRAM, 0);
        assert(write_sock >= 0);
        server = gethostbyname(ip.c_str());
        assert(server != NULL);
        memset(&write_addr, 0, sizeof(write_addr));
        write_addr.sin_family = AF_INET;
        memcpy(server->h_addr, &write_addr.sin_addr.s_addr, server->h_length);
        write_addr.sin_port = htons(port);
        int ret = connect(write_sock, (struct sockaddr *) &write_addr,
                sizeof(write_addr));
        assert(ret >= 0);

        read_sock = socket(AF_INET, SOCK_DGRAM, 0);
        server = gethostbyname("0.0.0.0");
        assert(server != NULL);
        memset(&read_addr, 0, sizeof(read_addr));
        read_addr.sin_family = AF_INET;
        memcpy(server->h_addr, &read_addr.sin_addr.s_addr, server->h_length);
        read_addr.sin_port = htons(port);
        ret = connect(read_sock, (struct sockaddr *) &read_addr,
                sizeof(read_addr));
        assert(ret >= 0);
    }

    void Send(const Packet& packet) {
        int written = write(write_sock, (const void*) &packet,
                packet.GetSize());
        assert(written == packet.GetSize());
    }

    bool Receive(Packet& packet) {
        int r = read(read_sock, (void*) &packet, sizeof(packet));
        assert(r > 0);
        return true;
    }

    unsigned GetTicks() const {
        return 0;
    }

protected:
    int write_sock, read_sock;
};

Socket* Socket::CreateBroadcast(const std::string& broadcastIP, uint16_t port) {
    return new UnixSocket(broadcastIP, port, true);
}

Socket* Socket::CreateStream(const std::string& ip, uint16_t port) {
    return new UnixSocket(ip, port, false);
}
}
