#include<bits/stdc++.h>
#include "random.hpp"
#include "coordinate.hpp"
#include "ant.hpp"
#include "data.hpp"

const int RADIUS = 1;

struct World {
    int N, M;
    int ant_number, item_number;
    std::vector<std::vector<int>> ant_map, item_map, carrying_map;
    std::vector<Ant> ants;
    std::vector<Data> items;

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

