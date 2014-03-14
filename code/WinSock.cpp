#ifdef WIN32

#include "Socket.h"
#include <iostream>
#include <cassert>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Packet.h"

#pragma comment(lib, "Ws2_32.lib")

namespace lifx {
	class Winsock : public Socket {
	public:
		Winsock(const std::string& ip, uint16_t port, bool broadcast) {
			WSADATA wsaData;

			assert( WSAStartup( MAKEWORD(2, 2), &wsaData ) == NO_ERROR );
			if (broadcast) {
				assert ((socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != INVALID_SOCKET);
			} else {
				assert ((socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET);
			}

			from.sin_family = AF_INET;
			from.sin_addr.s_addr = inet_addr("0.0.0.0");
			from.sin_port = htons(port);

			assert(bind(socket_, (SOCKADDR*) &from, sizeof(from)) != SOCKET_ERROR);

			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = inet_addr( ip.c_str() );
			dest.sin_port = htons( port );

			if (!broadcast) {  
				assert(connect(socket_ ,(sockaddr*)&dest,sizeof(dest)) == 0);
			}
		}

		void Send(const Packet& packet) {
			int sent = sendto(socket_,(const char*) &packet , packet.GetSize(), 0, (SOCKADDR*) &dest, sizeof(dest));
			if (sent != packet.GetSize()) {
				std::cerr << WSAGetLastError() << "\n";
			}
			assert(sent == packet.GetSize());
		}

		bool Receive(Packet& packet) {
			unsigned long nBytesAvailable;
			if ( ioctlsocket(socket_, FIONREAD, &nBytesAvailable) != SOCKET_ERROR )
			{
				if (nBytesAvailable == 0) {
					return false;
				}
			}

			int fromLen = sizeof(from);
			int recvd = recvfrom(socket_,(char*) &packet, sizeof(packet), 0, (SOCKADDR*) &from, &fromLen);
			return true;
		}

		unsigned GetTicks() const {
			return GetTickCount();
		}

	protected:
		SOCKET socket_;
		sockaddr_in from;
		sockaddr_in dest;
	};

	Socket* Socket::CreateBroadcast(const std::string& broadcastIP, uint16_t port) {
		return new Winsock(broadcastIP, port, true);
	}

	Socket* Socket::CreateStream(const std::string& ip, uint16_t port) {
		return new Winsock(ip, port, false);
	}
}

#endif
