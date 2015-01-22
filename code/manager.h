#pragma once
#include <iostream>
#include <map>
#include <fstream>
#include "Packet.h"
#include "Socket.h"
#include "Color.h"

using namespace std;

namespace lifx {
    class Manager {
    public:
        Manager(const string& broadcastIP);

        void ReadBulbs(const string& fname);

        void WriteBulbs(const string& fname);

        void SetTimeout(unsigned to);

        void Initialize() ;

        void SetDefaultColor(Color::rgb& rgb, float brightness);

        void MainLoop();

        bool Update();

        void ListDevices();

        void SendTo(Packet& packet, const string& name);

        void SendTo(Packet& packet, const MacAddress& mac);

        void SendToAll(Packet& packet);

        void Broadcast(Packet& packet);

        void GetLightState();

        void SetColor(float r, float g, float b, float brightness);

        void SetColor(const Color::rgb& rgb, float brightness);

    protected:
        Color::rgb color;
        float brightness;
        unsigned timeout;

        void ReadPacket();

        void HandleNewPacket(const Packet& packet);

        shared_ptr<Socket> socket;
        map<string, MacAddress> bulbs;
        unsigned startTime;
    };
}
