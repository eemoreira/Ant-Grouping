#include<bits/stdc++.h>

std::mt19937 rng((uint32_t)std::chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return std::uniform_int_distribution<int>(l, r)(rng); }

struct Coordinate {
    int x, y;
    Coordinate(int _x = 0, int _y = 0) : x(_x), y(_y) {}
};

std::vector<Coordinate> random_coordinates(int WIDTH, int HEIGHT, int N) {
    assert(WIDTH > 0 && HEIGHT > 0);
    long long total = 1LL * WIDTH * HEIGHT;
    assert(N >= 0 && N <= total);

    std::vector<int> xs(WIDTH), ys(HEIGHT);
    iota(xs.begin(), xs.end(), 0);
    iota(ys.begin(), ys.end(), 0);

    shuffle(xs.begin(), xs.end(), rng);
    shuffle(ys.begin(), ys.end(), rng);

    std::vector<bool> used((size_t)total, false);
    std::vector<Coordinate> res;
    res.reserve(N);

    for (int xi = 0; xi < WIDTH && (int)res.size() < N; ++xi) {
        int x = xs[xi];
        int remaining = N - (int)res.size();
        int max_take = std::min(HEIGHT, remaining);
        int to_take = (max_take > 0) ? uniform(0, max_take) : 0;

        int taken = 0;
        for (int j = 0; j < HEIGHT && taken < to_take; ++j) {
            int y = ys[j];
            int id = y * WIDTH + x;
            if (!used[id]) {
                used[id] = true;
                res.emplace_back(x, y);
                ++taken;
            }
        }
    }

    if ((int)res.size() < N) {
        for (int y = 0; y < HEIGHT && (int)res.size() < N; ++y) {
            for (int x = 0; x < WIDTH && (int)res.size() < N; ++x) {
                int id = y * WIDTH + x;
                if (!used[id]) {
                    used[id] = 1;
                    res.emplace_back(x, y);
                }
            }
        }
    }

    return res;
}

struct Ant {
    int x, y, vision_radius;
    Ant(int _x, int _y, int _vision_radius) : x(x), y(y), vision_radius(_vision_radius) {}

    void random_ste() {
        x += uniform(-1, 1);
        y += uniform(-1 , 1);
    }
};

struct Matrix {
    int N, M;
    int ant_number, item_number;
    std::vector<std::vector<int>> ant_map, item_map;

    Matrix(int _N, int _M, int ants, int items) 
        : N(_N), M(_M), ant_number(ants), item_number(items) {

        item_map.assign(N, std::vector<int>(M, false)); 
        ant_map.assign(N, std::vector<int>(M, false)); 

        for (Coordinate cord : random_coordinates(N, M, ants)) {
            ant_map[cord.x][cord.y] = true;
        }
        for (Coordinate cord : random_coordinates(N, M, items)) {
            item_map[cord.x][cord.y] = true;
        }
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (item_map[i][j] && ant_map[i][j]) {
                    std::cerr << 'B';
                } else if (item_map[i][j]) {
                    std::cerr << '#';
                } else if (ant_map[i][j]) {
                    std::cerr << 'A';
                } else {
                    std::cerr << '.';
                }
            }
            std::cerr << std::endl;
        }
    }
};

signed main() {
    Matrix mat(10, 10, 20, 30);
}

