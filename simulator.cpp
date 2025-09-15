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

const int RADIUS = 1;

struct World {
    int N, M;
    int ant_number, item_number;
    std::vector<std::vector<int>> ant_map, item_map, carrying_map;
    std::vector<Ant> ants;

    World(int _N, int _M, int _ants, int _items) 
        : N(_N), M(_M), ant_number(_ants), item_number(_items) {

        item_map.assign(N, std::vector<int>(M, false)); 
        ant_map.assign(N, std::vector<int>(M, 0)); 
        carrying_map.assign(N, std::vector<int>(M, 0)); 
        ants.reserve(ant_number);

        for (Coordinate cord : random_coordinates(N, M, ant_number)) {
            ant_map[cord.x][cord.y] += 1;
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
        _x = _x < 0 ? BOUND_WIDTH - 1 : _x;;
        _x = _x >= BOUND_WIDTH ? 0 : _x;

        _y += uniform(-1 , 1);
        _y = _y < 0 ? BOUND_HEIGHT - 1 : _y;
        _y = _y >= BOUND_HEIGHT ? 0 : _y;

        return Coordinate(_x, _y);
    }

    bool valid(int i, int j) {
        return i >= 0 && i < N && j >= 0 && j < M;
    }

#ifdef LOCAL_DEBUG
    void print(int step_number = -1) {
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
    void print(int step_number = -1) {
        // escreve arquivo PGM binário (P5). Dimensões: M=width, N=height
        std::string fname;
        if (step_number >= 0) fname = "res/step_" + std::to_string(step_number) + ".pgm";
        else fname = "res/step_last.pgm";

        std::ofstream fout(fname, std::ios::binary);
        if (!fout) return; // falha ao abrir arquivo -> não faz nada

        // Header P5: width height maxval
        fout << "P5\n" << M << " " << N << "\n255\n";

        // Mapeamento de valores (um único byte por pixel):
        // both (item + ant) -> 0   (preto)
        // item only         ->  80 (escuro)
        // ant carrying      -> 180
        // ant only          -> 220 (claro)
        // empty             -> 255 (branco)

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                unsigned char v;

                if (carrying_map[i][j]) assert(ant_map[i][j]);

                if (item_map[i][j] && ant_map[i][j]) v = 0;
                else if (item_map[i][j]) v = 80;
                else if (carrying_map[i][j]) v = 180;
                else if (ant_map[i][j]) v = 220;
                else v = 255;
                fout.write(reinterpret_cast<char*>(&v), 1);
            }
        }
        fout.close();
    }
#endif

    void do_action(Ant& ant) {
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
                if (ant.carrying) carrying_map[ant.cord.x][ant.cord.y] += 1;
                else carrying_map[ant.cord.x][ant.cord.y] -= 1;
            }

        }
    }

    void step() {

        for (Ant& ant : ants) {
            do_action(ant);
        }

        for (Ant& ant : ants) {
            int x = ant.cord.x, y = ant.cord.y;

            if (ant.carrying) carrying_map[x][y] -= 1;
            ant_map[x][y] -= 1;

            ant.cord = random_step(x, y, N, M);
            x = ant.cord.x, y = ant.cord.y;

            ant_map[x][y] += 1;
            if (ant.carrying) carrying_map[x][y] += 1;
        }

    }

    Coordinate first_not_filled(Coordinate cord) {
        std::queue<Coordinate> q;
        q.emplace(cord);
        std::vector<Coordinate> to;
        std::vector vis(N, std::vector<bool>(M, false));
        while (q.size()) {
            Coordinate now = q.front();
            q.pop();
            vis[now.x][now.y] = true;
            if (!item_map[now.x][now.y]) {
                to.emplace_back(now);
                continue;
            }
            for (int i = -RADIUS; i <= RADIUS; i++) {
                for (int j = -RADIUS; j <= RADIUS; j++) {
                    int x = now.x + i, y = now.y + j;
                    if (valid(x, y) && !vis[x][y]) {
                        vis[x][y] = true;
                        q.emplace(x, y);
                    }
                }
            }
        }

        int rand = uniform(0, int(to.size() - 1));
        return to[rand];
    }

    void endstep() {
        for (Ant& ant : ants) {
            if (ant.carrying) {
                Coordinate now = first_not_filled(ant.cord);
                ant_map[ant.cord.x][ant.cord.y] -= 1;
                carrying_map[ant.cord.x][ant.cord.y] -= 1;
                ant_map[now.x][now.y] += 1;
                item_map[now.x][now.y] += 1;
            }
        }
    }

    void simulate(int STEPS, int print_every) {
        for (int i = 0; i <= STEPS; i++) {
            step();
            if (i % print_every == 0) {
                print(i);
            }
        }

        endstep();
    }
};

signed main() {
    //World world(4, 4, 1, 1);
    //world.simulate(2, 1);

    World world(40, 40, 100, 500);
    world.simulate(100000, 1000);

    //World world(5, 5, 4, 10);
    //world.simulate(10000, 100);
}

