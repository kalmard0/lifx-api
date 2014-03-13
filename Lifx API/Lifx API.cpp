#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include <map>
#include "Socket.h"
#include "Packet.h"

using namespace lifx;

class Manager {
public:
	Manager(const std::string& broadcastIP ){
		socket = std::shared_ptr<Socket>(Socket::CreateBroadcast(broadcastIP));
	}

	void Initialize() {
		bulbs.clear();
		Packet packet;
		packet.Initialize(PacketType::GetPanGateway);
		socket->Send(packet);
	}

	void Update() {
		ReadPacket();
	}

protected:
	void ReadPacket() {
		Packet packet;
		if (socket->Receive(packet)) {
			std::cout << packet.ToString() << "\n";
			HandleNewPacket(packet);
		}
	}

	void HandleNewPacket(const Packet& packet) {
		if (packet.GetType() == PacketType::PanGatewayState) {
			Payload::PanGatewayState state = packet.GetPanGatewayState();
			if (state.service == (int) Payload::PanGatewayState::Service::UDP) {
				MacAddress siteMac = packet.GetSiteMac();
				bool hasName = false;
				for (const auto& it : bulbs) {
					if (it.second == siteMac) {
						hasName = true;
						break;
					}
				}

				if (!hasName) {
					Packet newPacket;
					newPacket.Initialize(PacketType::GetBulbLabel);
					newPacket.SetSiteMac(siteMac);
					socket->Send(newPacket);
				}
			}
		} else if (packet.GetType() == PacketType::BulbLabel) {
			bulbs[packet.GetBulbLabel().label] = packet.GetSiteMac();
		}
	}

	std::shared_ptr<Socket> socket;
	std::map<std::string, MacAddress> bulbs;
};

int _tmain(int argc, _TCHAR* argv[])
{
	Manager manager("192.168.66.41");
	manager.Initialize();

	while (true)
	{
		manager.Update();
	}

	return 0;
}

