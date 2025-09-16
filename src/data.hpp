#pragma once
#include <math>
#include "coordinate.hpp"

struct Data {
    double x, y;
    int group;

    Data(double _x, double _y, int _group) : x(_x), y(_y), group(_group) {}

    double dist(Data oth) {
        double X = x - oth.x;
        double Y = y - oth.y;
        return sqrt(X*X + Y*Y);
    }
};
