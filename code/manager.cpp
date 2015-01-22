#include "Manager.h"

using namespace std;
namespace lifx {
    Manager::Manager(const string& broadcastIP) {
        brightness = 0;
        socket = shared_ptr < Socket
                > (Socket::CreateBroadcast(broadcastIP));
        startTime = socket->GetTicks();
        timeout = 0;
    }

    void Manager::ReadBulbs(const string& fname) {
        ifstream file(fname);
        while (file) {
            string name;
            string mac;
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

    void Manager::WriteBulbs(const string& fname) {
        ofstream file(fname);
        for (const auto& it : bulbs) {
            if (it.first.length() > 0 && !it.second.IsNull()) {
                file << it.first << " " << it.second.ToString() << "\n";
            }
        }
    }

    void Manager::SetTimeout(unsigned to) {
        timeout = to;
    }

    void Manager::Initialize() {
        bulbs.clear();
        startTime = socket->GetTicks();
        ListDevices();
    }

    void Manager::SetDefaultColor(Color::rgb& rgb, float brightness) {
        color = rgb;
        this->brightness = brightness;
    }

    void Manager::MainLoop() {
        while (Update());
    }

    bool Manager::Update() {
        ReadPacket();
        if (timeout > 0 && startTime + timeout < socket->GetTicks()) {
            return false;
        }

        return true;
    }

    void Manager::ListDevices() {
        Packet packet;
        packet.Initialize(PacketType::GetPanGateway);
        socket->Send(packet);
    }

    void Manager::SendTo(Packet& packet, const string& name) {
        packet.SetSiteMac(bulbs[name]);
        socket->Send(packet);
    }

    void Manager::SendTo(Packet& packet, const MacAddress& mac) {
        packet.SetSiteMac(mac);
        socket->Send(packet);
    }

    void Manager::SendToAll(Packet& packet) {
        for (const auto& it : bulbs) {
            SendTo(packet, it.first);
        }
    }

    void Manager::Broadcast(Packet& packet) {
        packet.SetSiteMac(MacAddress());
        socket->Send(packet);
    }

    void Manager::GetLightState() {
        Packet newPacket;
        newPacket.Initialize(PacketType::GetLightState);
        SendToAll(newPacket);
    }

    void Manager::SetColor(float r, float g, float b, float brightness) {
        Color::rgb rgb;
        rgb.r = r;
        rgb.g = g;
        rgb.b = b;
        SetColor(rgb, brightness);
    }

    void Manager::SetColor(const Color::rgb& rgb, float brightness) {
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

    void Manager::ReadPacket() {
        Packet packet;
        if (socket->Receive(packet)) {
            cout << packet.ToString() << "\n";
            HandleNewPacket(packet);
        }
    }

    void Manager::HandleNewPacket(const Packet& packet) {
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
}
