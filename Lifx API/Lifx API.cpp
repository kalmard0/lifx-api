#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include "Socket.h"
#include "Packet.h"

using namespace lifx;

int _tmain(int argc, _TCHAR* argv[])
{
	std::shared_ptr<Socket> broadcastSocket( Socket::CreateBroadcast("192.168.66.255"));
	Packet packet;
	packet.Initialize(PacketType::GetPanGateway);
	broadcastSocket->Send(packet);

	std::shared_ptr<Socket> tcpSocket;

	while (!tcpSocket.get())
	{
		if (broadcastSocket->Receive(packet)) {
			std::cout << "udp: ";
			std::cout << packet.ToString() << "\n";
			if (packet.GetType() == PacketType::PanGatewayState) {
				Payload::PanGatewayState state = packet.GetPanGatewayState();
				if (state.service == (int) Payload::PanGatewayState::Service::TCP) {
					
					tcpSocket = std::shared_ptr<Socket>(Socket::CreateStream("192.168.66.41"));
				}
			}
		}
	}

	packet.Initialize(PacketType::GetWifiInfo);
	tcpSocket->Send(packet);

	while (true) {
		if (tcpSocket->Receive(packet)) {
			std::cout << "tcp: ";
			std::cout << packet.ToString() << "\n";
		}
	}

	return 0;
}

