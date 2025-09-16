#pragma once
#include <math.h>
#include "coordinate.hpp"

struct Data {
    double x, y;
    int group;

    Data(double _x, double _y, int _group) : x(_x), y(_y), group(_group) {}
    Data() : x(0), y(0), group(0) {}

    double dist(Data oth) {
        double X = x - oth.x;
        double Y = y - oth.y;
        return sqrt(X*X + Y*Y);
    }
};
