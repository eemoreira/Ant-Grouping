#pragma once
#include "coordinate.hpp"
#include "data.hpp"

const double ALPHA = 8;
const double K1 = 0.1;
const double K2 = 1;

#define sq(x) ((x)*(x))

struct Ant {
    Coordinate cord;
    int radius;
    bool carrying;
    Data data;
    Ant(Coordinate _cord, int _vision_radius) : cord(_cord), radius(_vision_radius) {
        carrying = false;
    }

    bool action(std::vector<Data>&v, int box_size) {
        if (!carrying) return pickup(v, box_size);
        return drop(v, box_size);
    }

    bool pickup(const std::vector<Data>& v, int box_size) {
        if (v.size() == 0) return false;

        double sum = 0;
        for(const Data& d : v) {
            sum += 1.0 - data.dist(d)/ALPHA;
        }

        double ssq = sq(v.size());

        double f = 1.0/ssq * sum;
        if(f <= 0) f = 0;

        double pp = sq(K1/(K1 + f));

        // pickup with pp prob
        return get_random() <= pp;
    }

    bool drop(std::vector<Data>&v, int box_size) {
        if (box_size - v.size() == 0) return false;

        double sum = 0;
        for(const Data& d : v) {
            sum += 1.0 - data.dist(d)/ALPHA;
        }

        double ssq = sq(v.size());

        double f = 1.0/ssq * sum;
        if(f <= 0) f = 0;

        double pd = sq(f/(K2 + f));

        // drop with pd prob
        return get_random() <= pd;
    }
};
