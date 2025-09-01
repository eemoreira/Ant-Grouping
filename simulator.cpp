#include<bits/stdc++.h>

std::mt19937 rng((uint32_t)std::chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return std::uniform_int_distribution<int>(l, r)(rng); }

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

        // drop with 1 - (empty_cord.size()) / box_size prob
        int rand = uniform(0, box_size - 1);
        return rand >= emp;
    }
};

const int RADIUS = 1;

struct World {
    int N, M;
    int ant_number, item_number;
    std::vector<std::vector<int>> ant_map, item_map;
    std::vector<Ant> ants;

    World(int _N, int _M, int _ants, int _items) 
        : N(_N), M(_M), ant_number(_ants), item_number(_items) {

        item_map.assign(N, std::vector<int>(M, false)); 
        ant_map.assign(N, std::vector<int>(M, false)); 
        ants.reserve(ant_number);

        for (Coordinate cord : random_coordinates(N, M, ant_number)) {
            ant_map[cord.x][cord.y] = true;
            ants.emplace_back(cord, RADIUS);
        }
        for (Coordinate cord : random_coordinates(N, M, item_number)) {
            item_map[cord.x][cord.y] = true;
        }
        print();
    }

    Coordinate random_step(int x, int y, int BOUND_WIDTH, int BOUND_HEIGHT) {
        int _x = x, _y = y;
        _x += uniform(-1, 1);
        _x = std::min(_x, BOUND_WIDTH - 1);
        _x = std::max(_x, 0);

        _y += uniform(-1 , 1);
        _y = std::min(_y, BOUND_HEIGHT - 1);
        _y = std::max(_y, 0);

        if (ant_map[_x][_y]) return Coordinate(x, y);
        return Coordinate(_x, _y);
    }

    bool valid(int i, int j) {
        return i >= 0 && i < N && j >= 0 && j < M;
    }

#ifdef LOCAL_DEBUG
    void print() {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (item_map[i][j] && ant_map[i][j]) {
                    std::cerr << '#';
                } else if (item_map[i][j]) {
                    std::cerr << '#';
                } else if (ant_map[i][j]) {
                    std::cerr << '.';
                } else {
                    std::cerr << '.';
                }
            }
            std::cerr << std::endl;
        }
        std::cerr << std::endl;
        std::cerr << std::endl;
    }
#else
    void print() {}
#endif

    void step() {
        for (Ant& ant : ants) {

            int cell_cnt = 0, filled_cnt = 0;
            for (int i = -ant.radius; i <= ant.radius; i++) {
                for (int j = -ant.radius; j <= ant.radius; j++) {
                    int xx = ant.cord.x + i, yy = ant.cord.y + j;
                    if (valid(xx, yy)) {
                        cell_cnt++;
                        if (item_map[xx][yy]) {
                            filled_cnt++;
                        }
                    }
                }
            }

            if (ant.action(filled_cnt, cell_cnt)) {
                if (ant.carrying ^ item_map[ant.cord.x][ant.cord.y]) {
                    item_map[ant.cord.x][ant.cord.y] ^= 1;
                    ant.carrying ^= 1;
                }
                //std::cerr << "filled_cnt = " << filled_cnt << ", cell_cnt = " << cell_cnt << ", cord = " << ant.cord << ", ant.carrying = " << (ant.carrying ? "True" : "False") << std::endl;
            }
        }

        for (Ant& ant : ants) {
            ant_map[ant.cord.x][ant.cord.y] = false;
            ant.cord = random_step(ant.cord.x, ant.cord.y, N, M);
            ant_map[ant.cord.x][ant.cord.y] = true;
        }

    }

    void simulate(int STEPS, int print_every) {
        for (int i = 0; i <= STEPS; i++) {
            step();
            if (i % print_every == 0) {
                print();
            }
        }
    }
};

signed main() {
    //World world(4, 4, 1, 1);
    //world.simulate(2, 1);

    World world(10, 10, 10, 20);
    world.simulate(1000000, 500000);
}

