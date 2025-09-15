#pragma once

#include "random.hpp"
#include <vector>

struct Coordinate {
    int x, y;
    Coordinate(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    friend std::ostream& operator << (std::ostream& out, Coordinate me) {
        return out << "(x = " << me.x << ", y = " << me.y << ")";
    }
};

std::vector<Coordinate> random_coordinates(int WIDTH, int HEIGHT, int N) {
    assert(WIDTH > 0 && HEIGHT > 0);
    long long total = 1LL * WIDTH * HEIGHT;
    assert(N >= 0 && N <= total);
    std::vector<bool> used(WIDTH*HEIGHT);
    std::vector<Coordinate> res;

    int tries = 10*N;
    while (tries-- && int(res.size()) < N) {
        int x = uniform(0, WIDTH - 1);
        int y = uniform(0, HEIGHT - 1);
        int id = y * WIDTH + x;
        if (used[id]) continue;
        used[id] = true;
        res.emplace_back(x, y);
    }

    if (int(res.size()) < N) {
        for (int y = 0; y < HEIGHT && (int)res.size() < N; ++y) {
            for (int x = 0; x < WIDTH && (int)res.size() < N; ++x) {
                int id = y * WIDTH + x;
                if (!used[id]) {
                    used[id] = true;
                    res.emplace_back(x, y);
                }
            }
        }
    }

    return res;
}

