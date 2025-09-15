#pragma once
#include "coordinate.hpp"

struct Ant {
    Coordinate cord;
    int radius;
    bool carrying;
    Ant(Coordinate _cord, int _vision_radius) : cord(_cord), radius(_vision_radius) {
        carrying = false;
    }

    bool action(int filled, int box_size) {
        if (!carrying) return pickup(filled, box_size);
        return drop(box_size - filled, box_size);
    }

    bool pickup(int filled, int box_size) {
        if (filled == 0) return false;

        // pickup with 1 - filled / box_size prob
        int rand = uniform(0, box_size - 1);
        return rand >= filled;
    }

    int drop(int emp, int box_size) {
        if (emp == 0) return false;
        bool ok = true;
        // drop with 1 - emp / box_size prob
        for (int i = 0; i < 2; i++) {
            int rand = uniform(0, box_size - 1);
            ok &= rand >= emp;
        }
        return ok;
    }
};
