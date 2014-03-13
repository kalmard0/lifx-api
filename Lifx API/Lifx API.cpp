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
		brightness = 0;
		socket = std::shared_ptr<Socket>(Socket::CreateBroadcast(broadcastIP));
	}

	void Initialize() {
		bulbs.clear();
		lastStatusCheck = socket->GetTicks();
		ListDevices();
	}

	void SetDefaultColor(Color::rgb& rgb, float brightness) {
		color = rgb;
		this->brightness = brightness;
	}

	void Update() {
		ReadPacket();
		if (lastStatusCheck + 10000 < socket->GetTicks()) {
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

	void Broadcast(Packet& packet) {
		packet.SetSiteMac(MacAddress());
		socket->Send(packet);
	}

	void GetLightState() {
		Packet newPacket;
		newPacket.Initialize(PacketType::GetLightState);
		SendToAll(newPacket);
	}

#if 0
	void SetDim(float dim) {
		Payload::SetDim setDim;
		setDim.duration = 1;
		setDim.brightness = dim * 0xff;
		Packet p;
		p.SetDim(setDim);
		SendToAll(p);
	}
#endif

	void SetColor(float r, float g, float b, float brightness) {
		Color::rgb rgb;
		rgb.r = r;
		rgb.g = g;
		rgb.b = b;
		SetColor(rgb, brightness);
	}

	void SetColor(const Color::rgb& rgb, float brightness) {
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
		Color::hsv hsv = Color::rgb2hsv(rgb);
		lc.hue = (uint8_t)( hsv.h / 360.f * 0xff);
		lc.saturation = (uint8_t)(hsv.s * 0xff);
		//lc.brightness = (uint8_t) (hsv.v * 0xff);
		lc.brightness = (uint8_t) (brightness * 0xff);
		packet.SetLightColorHSL(lc);
#endif
		SendToAll(packet);
	}

protected:
	Color::rgb color;
	float brightness;

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
			if (brightness > 0) {
				SetColor(color, brightness);
			}
		} else if (packet.GetType() == PacketType::LightStatus) {
			bulbs[packet.GetLightStatus().bulb_label] = packet.GetSiteMac();
			if (brightness > 0) {
				SetColor(color, brightness);
			}
		}
	}

	std::shared_ptr<Socket> socket;
	std::map<std::string, MacAddress> bulbs;
	unsigned lastStatusCheck;
};

int _tmain(int argc, _TCHAR* argv[])
{
	Manager manager("192.168.66.255");
	manager.SetDefaultColor(Color::rgb(1,0,0), 0.2f);
	manager.Initialize();

	while (true)
	{
		manager.Update();
	}

	return 0;
}

