#pragma once
#include "coordinate.hpp"
#include "data.hpp"
#include <math.h>

const double ALPHA = 10;
const double K1 = 0.1;
const double K2 = 0.3;

#define sq(x) ((x)*(x))

struct Ant {
    Coordinate cord;
    int radius;
    bool carrying;
    Data data;
    Ant(Coordinate _cord, int _vision_radius) : cord(_cord), radius(_vision_radius) {
        carrying = false;
        data = Data();
    }

    bool action(std::vector<Data>&v, Data _data, int box_size) {
        if (!carrying) return pickup(v, _data);
        return drop(v, box_size);
    }

    double F(const std::vector<Data>& v, Data data_to_compare) {
        double sum = 0;

        for(const Data& d : v) {
            //std::cerr << std::setprecision(10) << data_to_compare.dist(d) << std::endl;
            sum += std::max<double>(0, 1.0 - data_to_compare.dist(d)/ALPHA);
        }

        double ssq = v.size();
        return std::max<double>(0, 1.0/ssq * sum);
    }

    bool pickup(const std::vector<Data>& v, Data data_to_take) {
        if (int(v.size()) == 0) return false;


        double f = F(v, data_to_take);
        double pp = sq(K1 / (K1 + f));


        //std::cerr << std::setprecision(10) << std::fixed << "f = " << f << ", pickup prob = " << pp << std::endl;
        // pickup with pp prob
        return get_random() < pp;
    }

    bool drop(std::vector<Data>&v, int box_size) {
        if (box_size - int(v.size()) == 0) return false;

        double f = F(v, data);
        double pd = sq(f / (K2 + f));

        //std::cerr << std::setprecision(10) << std::fixed << "f = " << f << ", drop prob = " << pd << std::endl;
        // drop with pd prob
        return get_random() < pd;
    }
};
