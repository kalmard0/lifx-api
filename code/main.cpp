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
#include "manager.h"

using namespace lifx;

int main(int argc, const char* argv[]) {
    Manager manager("192.168.66.255");
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

