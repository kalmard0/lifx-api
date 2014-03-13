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
#include "Color.h"

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
		lastStatusCheck = socket->GetTicks();
	}

	void Update() {
		ReadPacket();
		if (lastStatusCheck + 10000 < socket->GetTicks()) {
			lastStatusCheck = socket->GetTicks();
			Packet packet;
			packet.Initialize(PacketType::GetLightState);
			for (const auto& it: bulbs) {
				packet.SetSiteMac(it.second);
				socket->Send(packet);
			}
		}
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
					newPacket.Initialize(PacketType::GetLightState);
					newPacket.SetSiteMac(siteMac);
					socket->Send(newPacket);

					Payload::LightColor lc;
					lc.Initialize();
					
#if 0
					lc.stream = 0;
					lc.hue = 0xaa;
					lc.brightness = 0xff;
					lc.saturation = 0xff;

					Color::RGBFloats floats;
					floats.r = 0;
					floats.g = 1;
					floats.b = 0;
					Color c(floats);

					Color::HSLBytes hsl = c.GetAsHSL();
					lc.hue = hsl.h;
					lc.brightness = hsl.l;
					lc.saturation = hsl.s;
					lc.fade_time = 0;
					lc.kelvin = 0;
#else
					lc.blue = 0xff;
					lc.green = 0xff;
					lc.red = 0xff;
					lc.white = 0xff;
#endif			
					newPacket.SetLightColor(lc);
					newPacket.SetTargetMac(siteMac);
					newPacket.SetSiteMac(siteMac);
					socket->Send(newPacket);
				}
			}
		} else if (packet.GetType() == PacketType::BulbLabel) {
			bulbs[packet.GetBulbLabel().label] = packet.GetSiteMac();
		} else if (packet.GetType() == PacketType::LightStatus) {
			bulbs[packet.GetLightStatus().bulb_label] = packet.GetSiteMac();
		}
	}

	std::shared_ptr<Socket> socket;
	std::map<std::string, MacAddress> bulbs;
	unsigned lastStatusCheck;
};

int _tmain(int argc, _TCHAR* argv[])
{
	Manager manager("192.168.66.255");
	manager.Initialize();

	while (true)
	{
		manager.Update();
	}

	return 0;
}

