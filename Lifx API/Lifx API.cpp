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
		lastStatusCheck = socket->GetTicks();
		ListDevices();
	}

	void Update() {
		ReadPacket();
		if (lastStatusCheck + 1000 < socket->GetTicks()) {
			lastStatusCheck = socket->GetTicks();
			GetLightState();
		}
	}

	void ListDevices() {
		Packet packet;
		packet.Initialize(PacketType::GetPanGateway);
		socket->Send(packet);
	}

	void SendTo(Packet& packet, const std::string& name) {
		packet.SetSiteMac(bulbs[name]);
		socket->Send(packet);
	}

	void SendTo(Packet& packet, const MacAddress& mac){
		packet.SetSiteMac(mac);
		socket->Send(packet);
	}

	void SendToAll(Packet& packet) {
		for (const auto& it : bulbs) {
			SendTo(packet, it.first);
		}
	}

	void GetLightState() {
		Packet newPacket;
		newPacket.Initialize(PacketType::GetLightState);
		SendToAll(newPacket);
	}

	void SetColor(float r, float g, float b) {
		Packet packet;
#if 0
		Payload::LightColorRGBW lc;
		lc.Initialize();
		lc.blue = (uint16_t) (b * 0xff);
		lc.red = (uint16_t) (r * 0xff);
		lc.green = (uint16_t) (g * 0xff);
		packet.SetLightColorRGBW(lc);
#else
		Payload::LightColorHSL lc;
		lc.Initialize();
		Color::rgb rgb;
		rgb.r = r;
		rgb.g = g;
		rgb.b = b;
		Color::hsv hsv = Color::rgb2hsv(rgb);
		lc.hue = (uint8_t)( hsv.h / 360.f * 0xff);
		lc.saturation = (uint8_t)(hsv.s * 0xff);
		lc.brightness = (uint8_t) (hsv.v * 0xff);
		packet.SetLightColorHSL(lc);
#endif
		SendToAll(packet);
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
					Packet p;
					p.Initialize(PacketType::GetBulbLabel);
					SendTo(p, siteMac);
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

