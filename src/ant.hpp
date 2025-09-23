#pragma once
#include "coordinate.hpp"
#include "data.hpp"
#include <math.h>

const double ALPHA = 35.8029;
const double K1 = 0.03;
const double K2 = 0.03;

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

    bool action(std::vector<Data>&v, Data _data) {
        if (!carrying) return pickup(v, _data);
        return drop(v);
    }

    double F(const std::vector<Data>& v, Data data_to_compare) {
        double sum = 0;

        for(const Data& d : v) {
            //std::cerr << std::setprecision(10) << data_to_compare.dist(d)/ALPHA << std::endl;
            sum += std::max(0.0, 1.0 - data_to_compare.dist(d)/ALPHA);
        }

        double ssq = sq(8);
        return std::max<double>(0, 1.0/ssq * sum);
    }

    bool pickup(const std::vector<Data>& v, Data data_to_take) {


        double f = F(v, data_to_take);
        //double pp = sq(1 - (double)v.size() / 8.0);

        double pp = sq(K1 / (K1 + f));
        //std::cerr << std::setprecision(10) << std::fixed << "f = " << f << ", pickup prob = " << pp << std::endl;
        // pickup with pp prob
        bool pick = get_random() < pp;
        //if (pick) {
        //    std::cerr << std::setprecision(10) << std::fixed << "v.size() = " << v.size() << ", pickup prob = " << pp << std::endl;
        //}
        return pick;
    }

    bool drop(std::vector<Data>&v) {

        double f = F(v, data);

        double pd = sq(f / (K2 + f));
        //double pd = sq((double)v.size() / 8.0);

        //std::cerr << std::setprecision(10) << std::fixed << "f = " << f << ", drop prob = " << pd << std::endl;
        // drop with pd prob
        bool dp = get_random() < pd;
        //if (dp) {
        //    std::cerr << std::setprecision(10) << std::fixed << "v.size() = " << v.size() << ", drop prob = " << pd << std::endl;
        //}
        return dp;
    }
};
