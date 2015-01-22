#pragma once
#include <cstdint>
#include <limits>

// These functions taken from http://codesuppository.blogspot.hu/2012/06/hsl2rgb-convert-rgb-colors-to-hsl-or.html

namespace lifx {
namespace Color {

struct rgb {
    rgb() {
        r = 0;
        g = 0;
        b = 0;
    }

    rgb(float r, float g, float b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    float r;
    float g;
    float b;
};

struct hsv {
    float h;       // angle in degrees
    float s;       // percent
    float v;       // percent
};

hsv rgb2hsv(const rgb& in);

rgb hsv2rgb(const hsv& in);
}
}
