#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include <map>
#include <fstream>
#include <cstring>
#include "Socket.h"
#include "Packet.h"
#include "Color.h"

using namespace lifx;

class Manager {
public:
    Manager(const std::string& broadcastIP) {
        brightness = 0;
        socket = std::shared_ptr < Socket
                > (Socket::CreateBroadcast(broadcastIP));
        startTime = socket->GetTicks();
        timeout = 0;
    }

    void ReadBulbs(const std::string& fname) {
        std::ifstream file(fname);
        while (file) {
            std::string name;
            std::string mac;
            file >> name;
            if (file) {
                file >> mac;
                MacAddress addr;
                addr.FromString(mac);
                bulbs[name] = addr;
            }
        }

        file.close();
    }

    void WriteBulbs(const std::string& fname) {
        std::ofstream file(fname);
        for (const auto& it : bulbs) {
            if (it.first.length() > 0 && !it.second.IsNull()) {
                file << it.first << " " << it.second.ToString() << "\n";
            }
        }
    }

    void SetTimeout(unsigned to) {
        timeout = to;
    }

    void Initialize() {
        bulbs.clear();
        startTime = socket->GetTicks();
        ListDevices();
    }

    void SetDefaultColor(Color::rgb& rgb, float brightness) {
        color = rgb;
        this->brightness = brightness;
    }

    void MainLoop() {
        while (Update());
    }

    bool Update() {
        ReadPacket();
        if (timeout > 0 && startTime + timeout < socket->GetTicks()) {
            return false;
        }

        return true;
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

    void SendTo(Packet& packet, const MacAddress& mac) {
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
        lc.hue = (uint8_t)(hsv.h / 360.f * 0xff);
        lc.saturation = (uint8_t)(hsv.s * 0xff);
        //lc.brightness = (uint8_t) (hsv.v * 0xff);
        lc.brightness = (uint8_t)(brightness * 0xff);
        lc.fade_time = 1;
        packet.SetLightColorHSL(lc);
#endif
        SendToAll(packet);
    }

protected:
    Color::rgb color;
    float brightness;
    unsigned timeout;

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
            WriteBulbs("cache");
        } else if (packet.GetType() == PacketType::LightStatus) {
            bulbs[packet.GetLightStatus().bulb_label] = packet.GetSiteMac();
            if (brightness > 0) {
                SetColor(color, brightness);
            }
            WriteBulbs("cache");
        }
    }

    std::shared_ptr<Socket> socket;
    std::map<std::string, MacAddress> bulbs;
    unsigned startTime;
};

int main(int argc, const char* argv[]) {
    Manager manager("255.255.255.255");
    manager.Initialize();
    manager.ReadBulbs("cache");

    if (argc > 4) {
        // set color and quit
        float r, g, b, brightness;
        sscanf(argv[1], "%f", &r);
        sscanf(argv[2], "%f", &g);
        sscanf(argv[3], "%f", &b);
        sscanf(argv[4], "%f", &brightness);
        manager.SetColor(r, g, b, brightness);

        std::cout << r << " " << g << " " << b << " " << brightness << "\n";
    } else {
        manager.MainLoop();
    }

    return 0;
}

